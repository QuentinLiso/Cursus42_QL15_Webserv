/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:09 by qliso             #+#    #+#             */
/*   Updated: 2025/06/21 17:32:54 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "6_ClientConnection.hpp"


// 1 - Constructor destructor

ClientConnection::ClientConnection(Server& server, int fd, const ListeningSocket* const relatedListeningSocket)
        :   _server(server),
			_fd(fd),
			_relatedListeningSocket(relatedListeningSocket),
			_clientConnectionState(STATE_READING_HEADERS),
			_needEpollToProgress(true),
			_httpRequest(),
			_httpResolution(_httpRequest),
			_cgiOutBuffer("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nConnection: close\r\nContent-Length: 132\r\n\r\n"),
			_cgiOutFinished(false)
{}

ClientConnection::~ClientConnection(void) {}

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
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = true;
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
	Console::log(Console::DEBUG, "Calling handle recv error");
	if (_httpRequest.getRequestBuffer().empty())
	{
		Console::log(Console::ERROR, "Recv from client fd failed, sending 500 to client...");
		_httpResponse.setDefaultErrorPage(500, HttpResponse::ERROR_HEADER_PARSING); 
		_clientConnectionState = STATE_READY_TO_SEND;
		// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
		_needEpollToProgress = true;
	}
}



void	ClientConnection::handleRequestValidation(void)
{
	Console::log(Console::DEBUG, "Handle request validation");
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
	_needEpollToProgress = true;
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
	_needEpollToProgress = true;
}

void	ClientConnection::handleValidDelete(void)
{
	_httpResponse.setDeleteResponse(_httpResolution.getHttpResolutionStatusCode());
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = true;
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
	_needEpollToProgress = true;
}

void	ClientConnection::handlePrepareReadingBody(void)
{
	Console::log(Console::DEBUG, "Hello from prepare reading body lol");
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
	if (_httpResolution.getResolutionState() == HttpRequestResolution::RESOLUTION_VALID_PUT_STATIC)
	{
		_httpResponse.setPutStaticResponse(_httpResolution.getHttpResolutionStatusCode());
		_clientConnectionState = STATE_READY_TO_SEND;
		// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
		_needEpollToProgress = true;
	}
	else
	{
		_clientConnectionState = STATE_CGI_PREPARE;
		_needEpollToProgress = false;
	}
}

void	ClientConnection::handleReadingBodyInvalid(void)
{
	Console::log(Console::DEBUG, "Handle reading body invalid lol");
	_httpResponse.setCustomErrorPage(_httpRequest.getHttpRequestData().getHttpRequestDataStatusCode(), _httpResolution.getLocationConfig(), HttpResponse::ERROR_BODY_READING);
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = true;
}

void	ClientConnection::handleReadingBody(void)
{
	Console::log(Console::DEBUG, "Handle reading body");
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
	Console::log(Console::DEBUG, "Hello from CGI prepare out lol");
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
		_server.registerNewFdToEpoll(_cgiHandler.getOutputPipeRead(), EPOLLIN, EPOLL_CTL_ADD, this, FdType::FD_CGI_PIPE);
		_clientConnectionState = STATE_CGI_READY;
		_needEpollToProgress = true;
	}
	else
	{
		_server.registerNewFdToEpoll(_cgiHandler.getInputPipeWrite(), EPOLLOUT, EPOLL_CTL_ADD, this, FdType::FD_CGI_PIPE);
		_server.registerNewFdToEpoll(_cgiHandler.getOutputPipeRead(), EPOLLIN, EPOLL_CTL_ADD, this, FdType::FD_CGI_PIPE);
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
	_needEpollToProgress = true;
}

void	ClientConnection::handleCgiReady(int events, int fd, FdType::Type fdType)
{
	Console::log(Console::DEBUG, "Hello from handle cgi ready");
	if ((events & EPOLLOUT) && fdType == FdType::FD_CGI_PIPE)
	{
		switch (_cgiHandler.writeToCgiInput())
		{
			case CgiHandler::CGI_RUNNING:																					break ;
			case CgiHandler::CGI_WRITING_TO_INPUT_DONE :	_server.deregisterFdFromEpoll(_cgiHandler.getInputPipeWrite(), FdType::FD_CGI_PIPE); break ;
			case CgiHandler::CGI_RUNNING_ERROR :			handleCgiRunningError(); 										return ;
			default : 	break ;
		}
	}

	if ((events & EPOLLIN) && fdType == FdType::FD_CGI_PIPE)
	{
		switch (_cgiHandler.readFromCgiOutput())
		{
			case CgiHandler::CGI_RUNNING : 										 	break ;
			case CgiHandler::CGI_READING_FROM_OUTPUT_DONE :	_server.deregisterFdFromEpoll(_cgiHandler.getOutputPipeRead(), FdType::FD_CGI_PIPE); break ;
			case CgiHandler::CGI_RUNNING_ERROR :			handleCgiRunningError(); return ;
			default :	return ;
		}
	}
	Console::log(Console::DEBUG, "End of handleCgiReady");
	if (_cgiHandler.isCgiReadFromOutputComplete())
		handleCgiValid();
}

void	ClientConnection::handleCgiRunningError(void)
{
	int	status;
	waitpid(_cgiHandler.getCgiPid(), &status, WNOHANG);
		
	_server.deregisterFdFromEpoll(_cgiHandler.getInputPipeWrite(), FdType::FD_CGI_PIPE);
	_server.deregisterFdFromEpoll(_cgiHandler.getOutputPipeRead(), FdType::FD_CGI_PIPE);
	
	_httpResponse.setCustomErrorPage(502, _httpResolution.getLocationConfig(), HttpResponse::ERROR_CGI_EXECUTION);
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = true;
}

void	ClientConnection::handleCgiValid(void)
{
	Console::log(Console::DEBUG, "Hello from Cgi valid before waitpid");
	int	status;
	waitpid(_cgiHandler.getCgiPid(), &status, WNOHANG);
	Console::log(Console::DEBUG, "Hello from Cgi valid after waitpid");
	
	// if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
	// 	_httpResponse.setCustomErrorPage(502, _httpResolution.getLocationConfig(), HttpResponse::ERROR_CGI_EXECUTION);
	// else
	_httpResponse.setCgiResponse(200, _cgiHandler.getCgiCompleteOutputFilename(), _cgiHandler.getTotalCgiOutputSize(), _cgiHandler.getCgiOutputHeaders(), _httpResolution.getLocationConfig());
	
	_clientConnectionState = STATE_READY_TO_SEND;
	// _server.registerSingleFdToEpoll(_fd, EPOLLOUT, EPOLL_CTL_MOD, FdType::FD_CLIENT_CONNECTION);
	_needEpollToProgress = true;
}


void	ClientConnection::handleReadyToSend(void)
{
	Console::log(Console::DEBUG, "Handle sending response");
	TStr	headersToString = _httpResponse.headersToString();
	std::cout << headersToString << std::endl;

	sendStr(headersToString);
	switch (_httpResponse.getResponseBodyType())
	{
		case HttpResponse::BODY_STRING:				sendStr(_httpResponse.getResponseBodyStr());	break ;
		case HttpResponse::BODY_FILE_DESCRIPTOR:
		case HttpResponse::BODY_CGI:				sendFd(_httpResponse.getResponseBodyFd());		break ; 
		default :									break ;
	}
	_clientConnectionState = STATE_CLOSING_CONNECTION;
	_needEpollToProgress = true;
}

void	ClientConnection::sendStr(const TStr& str)
{
	ssize_t	bytesSent = 0;
	const char*	data = str.c_str();
	size_t	bytesToSend = str.size();

	while (bytesSent < static_cast<ssize_t>(bytesToSend))
	{
		ssize_t n = send(_fd, data + bytesSent, bytesToSend - bytesSent, 0);
		if (n < 0)
		{
			Console::log(Console::ERROR, "Send to client encountered an error");
			return ;
		}
		else if (n == 0)
		{
			Console::log(Console::ERROR, "Client closed connection unexpectedly");
			return ;
		}
		bytesSent += n;
	}
}

void	ClientConnection::sendFd(int fd)
{
	Console::log(Console::INFO, "Sending from fd " + convToStr(fd));
	char	buffer[4096];
	ssize_t	bytesRead = 1;
	ssize_t	bytesSent = 0;
	
	while (bytesRead > 0)
	{
		bytesRead = read(fd, buffer, sizeof(buffer));
		Console::log(Console::DEBUG, "Bytes read from send fd : " + convToStr(bytesRead));
		if (bytesRead < 0)
		{
			Console::log(Console::ERROR, "Reading body file encountered an error");
			break ;
		}

		while (bytesSent < bytesRead)
		{
			ssize_t n = send(_fd, buffer, bytesRead, 0);
			if (n <= 0)
			{
				Console::log(Console::ERROR, "Send to client encountered an error");
				close(fd);
				return ;
			}
			bytesSent += n;
		}
	}
	Console::log(Console::INFO, "Closing fd " + convToStr(fd));
	close(fd);
}




// Public
void	ClientConnection::handleEvent(int events, int fd, FdType::Type fdType)
{
	switch(_clientConnectionState)
	{
		case STATE_READING_HEADERS :		handleReadingHeaders();		break;
		case STATE_REQUEST_RESOLUTION :		handleRequestValidation();	break;
		case STATE_PREPARE_READING_BODY:	handlePrepareReadingBody();	break;
		case STATE_READING_BODY :			handleReadingBody(); 		break;
		case STATE_READY_TO_SEND:			handleReadyToSend(); 		break;


		case STATE_CGI_PREPARE :			handleCgiPrepare(); 		break;
		case STATE_CGI_READY :				handleCgiReady(events, fd, fdType);			break;
		case STATE_CGI_FINISHED :			return ;
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



// void	ClientConnection::handleWriteCgiToClient(void)
// {
// 	if (_cgiOutBuffer.empty() && _cgiOutFinished == false)
// 	{
// 		return ;
// 	}
// 	ssize_t	bytesSent = send(_fd, _cgiOutBuffer.c_str(), _cgiOutBuffer.size(), 0);
// 	Console::log(Console::DEBUG, "Hello from write to client");
	
// 	std::cout << bytesSent << std::endl;
// 	if (bytesSent < 0)
// 	{
// 		_httpResponse.setCustomErrorPage(502, _httpResolution.getLocationConfig(), HttpResponse::ERROR_REQUEST_RESOLUTION);
// 		_clientConnectionState = STATE_CGI_RUNNING_ERROR;
// 		_needEpollToProgress = false;
// 	}
// 	else if (bytesSent == 0)
// 	{
// 		Console::log(Console::ERROR, "Client closed connection unexpecteddddddddddly");
// 		_clientConnectionState = STATE_CGI_VALID;
// 		_needEpollToProgress = false;
// 	}
// 	_cgiOutBuffer.erase(0, static_cast<size_t>(bytesSent));
// }




// void	ClientConnection::handleWriteToCgiStdin(void)
// {
// 	char	buffer[8192];

// 	ssize_t bytesRead = read(_cgiHandler.getRequestBodyInputFd(), buffer, sizeof(buffer));
// 	if (bytesRead < 0)
// 	{
// 		_httpResponse.setCustomErrorPage(502, _httpResolution.getLocationConfig(), HttpResponse::ERROR_REQUEST_RESOLUTION);
// 		_clientConnectionState = STATE_CGI_RUNNING_ERROR;
// 		_needEpollToProgress = false;
// 		return ;
// 	}
// 	else if (bytesRead == 0)
// 	{
// 		return ;
// 	}
// 	ssize_t	totalBytesWritten = 0;
// 	while (totalBytesWritten < bytesRead)
// 	{
// 		ssize_t bytesWritten = write(_cgiHandler.getInputPipeWrite(), buffer + totalBytesWritten, bytesRead - totalBytesWritten);
// 		if (bytesWritten < 0)
// 		{
// 			_httpResponse.setCustomErrorPage(502, _httpResolution.getLocationConfig(), HttpResponse::ERROR_REQUEST_RESOLUTION);
// 			_clientConnectionState = STATE_CGI_RUNNING_ERROR;
// 			_needEpollToProgress = false;
// 			return ;
// 		}
// 		totalBytesWritten += bytesWritten;
// 	}
// }

// void	ClientConnection::handleReadFromCgiStdout(void)
// {
// 	Console::log(Console::DEBUG, "Hello from read from cgi stdout");

// 	char	buffer[8192];

// 	ssize_t bytesRead = read(_cgiHandler.getOutputPipeRead(), buffer, sizeof(buffer));
// 	if (bytesRead < 0)
// 	{
// 		_httpResponse.setCustomErrorPage(502, _httpResolution.getLocationConfig(), HttpResponse::ERROR_REQUEST_RESOLUTION);
// 		_clientConnectionState = STATE_CGI_RUNNING_ERROR;
// 		_needEpollToProgress = false;
// 		return ;
// 	}
// 	else if (bytesRead == 0)
// 	{
// 		_cgiOutFinished = true;
// 		return ;
// 	}
// 	_cgiOutBuffer.append(buffer, bytesRead);
// 	if (static_cast<size_t>(bytesRead) < sizeof(buffer))
// 		_cgiOutFinished = true;

// 	std::cout << _cgiOutBuffer << std::endl;

// }

