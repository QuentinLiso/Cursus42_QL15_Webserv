/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:09 by qliso             #+#    #+#             */
/*   Updated: 2025/06/22 22:58:44 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "6_ClientConnection.hpp"

// Static
int 	ClientConnection::filenum = 0;

// 1 - Constructor destructor

ClientConnection::ClientConnection(Server& server, int fd, const ListeningSocket* const relatedListeningSocket)
        :   _server(server),
			_fd(fd),
			_relatedListeningSocket(relatedListeningSocket),
			_clientConnectionState(STATE_READING_HEADERS),
			_needEpollToProgress(true),
			_httpRequest(),
			_httpResolution(_httpRequest),
			_sendState(SENDING_HEADERS),
			_sendOffset(0),
			_sendFd(-1),
			_actualBytesSent(0),
			_sendFdClear(false)
{
	std::ostringstream oss;
	oss << "/home/qliso/Documents/Webserv_github/html/tmp/recv_tmp_" << ClientConnection::filenum++;
	logfile.open(oss.str().c_str(), std::ios::out | std::ios::binary);
}

ClientConnection::~ClientConnection(void)
{
	logfile.close();
}



// Private
void	ClientConnection::handleReadingHeaders(void)
{
	Console::log(Console::DEBUG, "Handle reading headers");
	char	recvBuffer[8192];
	
	ssize_t	bytesRead = recv(_fd, recvBuffer, sizeof(recvBuffer), MSG_DONTWAIT);
	if (bytesRead > 0)
	{
		switch (_httpRequest.parseHttpRequest(recvBuffer, bytesRead))
		{
			case	HttpRequest::PARSING_HEADERS_NEED_DATA :	return ;
			case	HttpRequest::PARSING_HEADERS_DONE :			handleReadingHeadersDone(); return;
			default :											handleReadingHeadersInvalid();	return ;
		}
	}
	
	else if (bytesRead == 0)
		handleClientDisconnectedWhileRecv();
	
	else
		handleRecvError();
}

void	ClientConnection::handleReadingHeadersInvalid(void)
{
	_httpResponse.setDefaultErrorPage(_httpRequest.getHttpRequestData().getHttpRequestDataStatusCode(), HttpResponse::ERROR_HEADER_PARSING); 
	_clientConnectionState = STATE_READY_TO_SEND;
	_needEpollToProgress = false;
}

void	ClientConnection::handleReadingHeadersDone(void)
{
	_clientConnectionState = STATE_REQUEST_RESOLUTION;
	_needEpollToProgress = false;
}

void	ClientConnection::handleClientDisconnectedWhileRecv(void)
{
	Console::log(Console::WARNING, "Client disconnected before request was complete");
	_clientConnectionState = STATE_CLOSING_CONNECTION;
	_needEpollToProgress = true;
}

void	ClientConnection::handleRecvError(void)
{
	// Do nothing actually lol
}



void	ClientConnection::handleRequestValidation(void)
{
	switch (_httpResolution.resolveHttpRequest(_relatedListeningSocket))
	{
		case HttpRequestResolution::RESOLUTION_VALID_GET_HEAD_STATIC :		handleValidGetHeadStatic();		return ;
		case HttpRequestResolution::RESOLUTION_VALID_GET_HEAD_CGI :			handleValidGetHeadCgi(); 		return ;
		case HttpRequestResolution::RESOLUTION_VALID_GET_HEAD_AUTOINDEX :	handleValidGetHeadAutoindex();	return ;
		case HttpRequestResolution::RESOLUTION_VALID_DELETE :				handleValidDelete(); 			return;
		case HttpRequestResolution::RESOLUTION_VALID_POST_CGI :			
		case HttpRequestResolution::RESOLUTION_VALID_PUT_STATIC :	
		case HttpRequestResolution::RESOLUTION_VALID_PUT_CGI :				handleValidPostPut(); 			return;
		default :															handleInvalidRequest(); 		return;
	}
}

void	ClientConnection::handleValidGetHeadStatic(void)
{
	Console::log(Console::DEBUG, "Valid GET HEAD static");
	_httpResponse.setGetHeadStaticResponse(	_httpResolution.getHttpResolutionStatusCode(), 
											_httpResolution.getResolvedPath(),
											_httpResolution.getResolvedPathStat(),
											_httpResolution.getLocationConfig(),
											_httpRequest.getHttpRequestData().getMethod());
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = false;
}

void	ClientConnection::handleValidGetHeadCgi(void)
{
	_clientConnectionState = STATE_CGI_PREPARE;
	_cgiHandler.setOutOnly(true);
	_needEpollToProgress = false;
}

void	ClientConnection::handleValidGetHeadAutoindex(void)
{
	_httpResponse.setGetHeadAutoindexResponse(	_httpResolution.getHttpResolutionStatusCode(),
												_httpResolution.getResolvedPath(),
												_httpResolution.getLocationConfig(),
												_httpRequest.getHttpRequestData().getMethod());
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = false;
}

void	ClientConnection::handleValidDelete(void)
{
	_httpResponse.setDeleteResponse(_httpResolution.getHttpResolutionStatusCode());
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = false;
}

void	ClientConnection::handleValidPostPut(void)
{
	_clientConnectionState = STATE_PREPARE_READING_BODY;
	_cgiHandler.setOutOnly(false);
	_needEpollToProgress = false;
}

void	ClientConnection::handleInvalidRequest(void)
{
	_httpResponse.setCustomErrorPage(_httpResolution.getHttpResolutionStatusCode(), _httpResolution.getLocationConfig(), HttpResponse::ERROR_REQUEST_RESOLUTION);
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = false;
}

void	ClientConnection::handlePrepareReadingBody(void)
{
	switch (_httpRequest.prepareParseHttpBody(_httpResolution.getLocationConfig(), _httpResolution.getResolutionState() == HttpRequestResolution::RESOLUTION_VALID_PUT_STATIC, _httpResolution.getResolvedPath()))
	{
		case	HttpRequest::PARSING_BODY_CONTENT_LENGTH_NEED_DATA:
		case	HttpRequest::PARSING_CHUNK_SIZE:
		case	HttpRequest::PARSING_CHUNK_DATA:
		case	HttpRequest::PARSING_CHUNK_LAST_CRLF:	handlePrepareReadingBodyNeedsData();	return ;
		case	HttpRequest::PARSING_BODY_DONE:			handleParsingBodyDone();				return ;
		default:										handleReadingBodyInvalid();				return ;
	}
}

void	ClientConnection::handlePrepareReadingBodyNeedsData(void)
{
	_clientConnectionState = STATE_READING_BODY;
	_needEpollToProgress = true;		// We parsed the trailing request buffer but couldn't after a complete body so need another EPOLLIN event
}

void	ClientConnection::handleParsingBodyDone(void)
{
	std::cout << "[PARISNG DONE] Total body size : " << _httpRequest.getActualChunkedDataSize() << std::endl;
	if (_httpResolution.getResolutionState() == HttpRequestResolution::RESOLUTION_VALID_PUT_STATIC)
	{
		_httpResponse.setPutStaticResponse(_httpResolution.getHttpResolutionStatusCode());
		_clientConnectionState = STATE_READY_TO_SEND;
		// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
		_needEpollToProgress = false;
	}
	else
	{
		_clientConnectionState = STATE_CGI_PREPARE;
		_needEpollToProgress = false;
	}
}

void	ClientConnection::handleReadingBodyInvalid(void)
{
	_httpResponse.setCustomErrorPage(_httpRequest.getHttpRequestData().getHttpRequestDataStatusCode(), _httpResolution.getLocationConfig(), HttpResponse::ERROR_BODY_READING);
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = false;
}

void	ClientConnection::handleReadingBody(void)
{
	// Console::log(Console::DEBUG, "Handle reading body");
	char	recvBuffer[32 * 1024];
	
	ssize_t	bytesRead = recv(_fd, recvBuffer, sizeof(recvBuffer), MSG_DONTWAIT);
	if (bytesRead > 0)
	{
		switch (_httpRequest.parseHttpBody(recvBuffer, bytesRead))
		{
			case	HttpRequest::PARSING_BODY_CONTENT_LENGTH_NEED_DATA :
			case	HttpRequest::PARSING_CHUNK_SIZE:
			case	HttpRequest::PARSING_CHUNK_DATA:
			case	HttpRequest::PARSING_CHUNK_LAST_CRLF:					return ;
			case	HttpRequest::PARSING_BODY_DONE:							handleParsingBodyDone();	return ;
			default	:														handleReadingBodyInvalid();	return ;
		}
	}
	else if (bytesRead == 0)
		handleClientDisconnectedWhileRecv();

	else
		handleRecvError();
}

void	ClientConnection::handleCgiPrepare(void)
{
	switch (_cgiHandler.setupCgi(_httpRequest, _httpResolution))
	{
		case CgiHandler::CGI_SETUP_VALID :	handleCgiPrepareValid();	return ;
		default :							handleCgiPrepareError(); 	return ;
	}
}

void	ClientConnection::handleCgiPrepareValid(void)
{
	if (_cgiHandler.isOutOnly())
	{
		_server.registerNewFdToEpoll(_cgiHandler.getOutputPipeRead(), EPOLLIN | EPOLLHUP, EPOLL_CTL_ADD, this, FdType::FD_CGI_PIPE);
		_clientConnectionState = STATE_CGI_READY;
		_needEpollToProgress = true;
	}
	else
	{
		_server.registerNewFdToEpoll(_cgiHandler.getInputPipeWrite(), EPOLLOUT | EPOLLHUP, EPOLL_CTL_ADD, this, FdType::FD_CGI_PIPE);
		_server.registerNewFdToEpoll(_cgiHandler.getOutputPipeRead(), EPOLLIN | EPOLLHUP, EPOLL_CTL_ADD, this, FdType::FD_CGI_PIPE);
		_clientConnectionState = STATE_CGI_READY;
		_needEpollToProgress = true;
	}
}

void	ClientConnection::handleCgiPrepareError(void)
{
	Console::log(Console::DEBUG, "Hello from CGI prepare error lol");
	_httpResponse.setCustomErrorPage(500, _httpResolution.getLocationConfig(), HttpResponse::ERROR_REQUEST_RESOLUTION);
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = false;
}

void	ClientConnection::handleCgiReady(int events, int fd, FdType::Type fdType)
{
	static int i = 0;
	if ((events & EPOLLOUT) && fdType == FdType::FD_CGI_PIPE)
	{
		if (_cgiHandler.writeToCgiInputPipe())
		{
			std::cout << "Writing to cgi input pipe was true : " << _cgiHandler.getActualBytesWrittenToCgiInput() << std::endl;
			close(_cgiHandler.getInputPipeWrite());
		}
	}

	if ((events & EPOLLIN) && fdType == FdType::FD_CGI_PIPE)
	{
		if(_cgiHandler.readFromCgiOutputPipe())
		{
			std::cout << "Reading from cgi output pipe was true" << std::endl;
		}
	}

	if ((events & EPOLLHUP) && fdType == FdType::FD_CGI_PIPE)
	{
		std::cout << "[DEBUG] EPOLLHUP detected on CGI pipe" << std::endl;

		int status;
		pid_t result = waitpid(_cgiHandler.getCgiPid(), &status, WNOHANG);
		if (result == _cgiHandler.getCgiPid()) {
			_needEpollToProgress = false;
			_clientConnectionState = STATE_CGI_FINISHED;
			std::cout << "[INFO] CGI process exited, moving to finished state." << std::endl;
		}
	}

}

void	ClientConnection::handleCgiFinished(void)
{
	std::cout << "Called" << std::endl;
	_cgiHandler.flushBuffer();
	std::cout << "Reading from cgi output pipe was true : " << _cgiHandler.getActualBytesReadFromCgiOutput() << std::endl;
	close(_cgiHandler.getOutputPipeRead());
	_httpResponse.setCgiResponse(_cgiHandler.getCgiStatusCode(), _cgiHandler.getCgiCompleteOutputFilename(), _cgiHandler.getActualBytesReadFromCgiOutput(), _cgiHandler.getCgiOutputHeaders(), _httpResolution.getLocationConfig());
	_clientConnectionState = STATE_READY_TO_SEND;
	_needEpollToProgress = false;
}


void	ClientConnection::handleReadyToSend(void)
{
	_sendBuffer = _httpResponse.headersToString();
	if (_httpResponse.getResponseBodyType() == HttpResponse::BODY_STRING)
	{
		_sendBuffer += _httpResponse.getResponseBodyStr();
		_clientConnectionState = STATE_SENDING_BODY_STR;
		_needEpollToProgress = true;
	}
	else
	{
		_sendFd = _httpResponse.getResponseBodyFd();
		_clientConnectionState = STATE_SENDING_BODY_FD;
		_needEpollToProgress = true;
	}
}

void	ClientConnection::handleSendingStr(int events, FdType::Type fdType)
{
	if ((events & EPOLLOUT) && fdType == FdType::FD_CLIENT_CONNECTION)
	{
		while (!_sendBuffer.empty())
		{
			ssize_t	bytesSent = send(_fd, _sendBuffer.c_str(), _sendBuffer.size(), MSG_NOSIGNAL);
			if (bytesSent < 0)
			{
				Console::log(Console::ERROR, strerror(errno));
				return ;
			}
			else if (bytesSent == 0)
				break ;
			_sendBuffer.erase(0, bytesSent);
			_actualBytesSent += bytesSent;
		}
		close (_fd);
		_clientConnectionState = STATE_CLOSING_CONNECTION;
		_needEpollToProgress = true;
	}
	if ((events & EPOLLHUP) && fdType == FdType::FD_CLIENT_CONNECTION)
	{
		close (_fd);
		_clientConnectionState = STATE_CLOSING_CONNECTION;
		_needEpollToProgress = true;
	}
}

void	ClientConnection::handleSendingFd(int events, FdType::Type fdType)
{
	if ((events & EPOLLOUT) && fdType == FdType::FD_CLIENT_CONNECTION)
	{
		while (!_sendBuffer.empty())
		{
			ssize_t	bytesSent = send(_fd, _sendBuffer.c_str(), _sendBuffer.size(), MSG_DONTWAIT);
			if (bytesSent < 0)
			{
				Console::log(Console::ERROR, strerror(errno));
				close (_fd);
				_clientConnectionState = STATE_CLOSING_CONNECTION;
				_needEpollToProgress = true;
				std::cout << "Total bytes sent 1 : " << _actualBytesSent << std::endl;
				return ;
			}
			else if (bytesSent == 0)
			{
				break ;
			}
			_sendBuffer.erase(0, bytesSent);		// buffer empty or not we don't know yet
			_actualBytesSent += bytesSent;
			if (!_sendBuffer.empty())
			{
				std::cout << "Total bytes sent 2 : " << _actualBytesSent << std::endl;
				return ;
			}
		}

		if (!_sendFdClear)
		{
			char	buffer[1024 * 8];
			ssize_t	bytesRead = read(_sendFd, buffer, sizeof(buffer));
			if (bytesRead < 0)
			{
				Console::log(Console::ERROR, strerror(errno));
				std::cout << "Total bytes sent 3 bytes read < 0 while reading fd : " << _actualBytesSent << std::endl;
			}
			else if (bytesRead == 0)
			{
				_sendFdClear = true;
				close(_sendFd);
			}
			else
			{
				_sendBuffer.append(buffer, bytesRead);
			}
		}

		if (_sendFdClear && _sendBuffer.empty())
		{
			std::cout << "Total bytes sent 4 : " << _actualBytesSent << std::endl;
			close (_fd);
			_clientConnectionState = STATE_CLOSING_CONNECTION;
			_needEpollToProgress = true;
		}

	}
	if ((events & EPOLLHUP) && fdType == FdType::FD_CLIENT_CONNECTION)
	{
		std::cout << "Total bytes sent 5 : " << _actualBytesSent << std::endl;
		close (_fd);
		_clientConnectionState = STATE_CLOSING_CONNECTION;
		_needEpollToProgress = true;
	}
}


// Public
void	ClientConnection::handleEvent(int events, int fd, FdType::Type fdType)
{
	switch(_clientConnectionState)
	{
		case STATE_READING_HEADERS :		if ((events & EPOLLIN) != 0) handleReadingHeaders();		break;
		case STATE_REQUEST_RESOLUTION :		handleRequestValidation();	break;
		case STATE_PREPARE_READING_BODY:	handlePrepareReadingBody();	break;
		case STATE_READING_BODY :			handleReadingBody(); 		break;
		case STATE_READY_TO_SEND:			handleReadyToSend(); 		break;
		case STATE_SENDING_BODY_STR :		handleSendingStr(events, fdType);			break;
		case STATE_SENDING_BODY_FD :		handleSendingFd(events, fdType);			break;
		
		case STATE_CGI_PREPARE :			handleCgiPrepare(); 		break;
		case STATE_CGI_READY :				handleCgiReady(events, fd, fdType);			break;
		case STATE_CGI_FINISHED :			handleCgiFinished() ;	return ;
		case STATE_CLOSING_CONNECTION: 		return;
		default:							return;
	}
}

int								ClientConnection::getFd(void) const { return _fd; }
ClientConnection::ClientState	ClientConnection::getClientConnectionState(void) const { return _clientConnectionState; }
bool							ClientConnection::needEpollEventToProgress(void) const { return _needEpollToProgress; }
const CgiHandler&				ClientConnection::getCgiHandler(void) const { return _cgiHandler; }


void	ClientConnection::setClientConnectionSendingResponse(void) { _clientConnectionState = STATE_SENDING_RESPONSE; }

void	ClientConnection::setClientConnectionCgiReady(void) { _clientConnectionState = STATE_CGI_READY; }



// bool	ClientConnection::sendFromFdDone(void)
// {
// 	char	buffer[8192];

// 	ssize_t	bytesSent = 0;
// 	if (!_sendBuffer.empty())
// 	{
// 		_sendOffset = 0;
// 		while (_sendOffset < _sendBuffer.size())
// 		{
// 			bytesSent = send(_fd, _sendBuffer.c_str() + _sendOffset, _sendBuffer.size() - _sendOffset, 0);
// 			if (bytesSent <= 0)
// 				return (false);
// 			_sendOffset += bytesSent;
// 		}
// 		_actualBytesSent += _sendOffset;
// 		_sendBuffer.clear();
// 		_sendOffset = 0;
// 	}

// 	ssize_t	bytesRead = read(_sendFd, buffer, sizeof(buffer));
// 	if (bytesRead < 0)
// 		return (false);
// 	else if (bytesRead == 0)
// 		return (true);

// 	size_t	totalBytesSent = 0;
// 	while (totalBytesSent < static_cast<size_t>(bytesRead))
// 	{
// 		bytesSent = send(_fd, buffer + totalBytesSent, bytesRead - totalBytesSent, 0);
// 		if (bytesSent <= 0)
// 		{
// 			_sendBuffer.append(buffer, totalBytesSent, bytesRead - totalBytesSent);
// 			return (false);
// 		}
// 		totalBytesSent += bytesSent;
// 	}
// 	_actualBytesSent += totalBytesSent;
// 	return (false);
// }

