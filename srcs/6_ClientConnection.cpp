/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:09 by qliso             #+#    #+#             */
/*   Updated: 2025/06/14 16:12:26 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "6_ClientConnection.hpp"


// 1 - Constructor destructor

ClientConnection::ClientConnection(int fd, const ListeningSocket* relatedListeningSocket)
        :   _fd(fd),
			_relatedListeningSocket(relatedListeningSocket),
			_httpRequest()
{}

ClientConnection::~ClientConnection(void) {}


// 3 - Preparing response
HttpResponse::Status			ClientConnection::prepareResponseFromRequestHeadersComplete(void)
{
	const LocationConfig* loc = _relatedListeningSocket->findLocationConfig(_httpRequest.getHostAddress(), _httpRequest.getUriPath());
	std::cout << "CURRENT BUFFER :\n" << _httpRequest.getBuffer() << std::endl;
	HttpResponse::Status	ret = _httpResponse.prepareResponseFromRequestHeadersComplete(&_httpRequest, loc);
	
	_httpResponse.print();
	return (ret);
}

ClientConnection::ReadStatus	ClientConnection::prepareResponseFromRequestBodyComplete(void)
{
	return (READ_OK);
}

ClientConnection::ReadStatus	ClientConnection::prepareResponseInvalidRequest(unsigned short code, ClientConnection::ReadStatus readStatus)
{
	_httpResponse.prepareResponseInvalidRequest(code);
	_httpResponse.print();
	return (readStatus);
}


// 4 - Sending response

void	ClientConnection::sendResponseHeader(void)
{
	TStr	responseHeaders = _httpResponse.toString();
	send(_fd, responseHeaders.c_str(), responseHeaders.size(), 0);
}

void	ClientConnection::sendResponseBody(void)
{
	switch (_httpResponse.getBodyType())
	{
		case HttpResponse::NO_BODY:			break ;
		case HttpResponse::FILEDESCRIPTOR:	sendResponseBodyFd(_httpResponse.getBodyFd()); break ;
		case HttpResponse::STRING:			sendResponseBodyStr(_httpResponse.getBodyStr()); break;
		default :							break;
	}
}

void	ClientConnection::sendResponseBodyFd(int bodyfd)
{
	char	buffer[4096];
	ssize_t	bytesRead = 1;
	ssize_t	bytesSent = 0;
	
	while (bytesRead > 0)
	{
		bytesRead = read(bodyfd, buffer, sizeof(buffer));
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
				close(bodyfd);
				return ;
			}
			bytesSent += n;
		}
	}	
	close(bodyfd);
}

void	ClientConnection::sendResponseBodyStr(const TStr& bodystr)
{
	ssize_t	bytesSent = 0;
	const char*	data = bodystr.c_str();
	size_t	bytesToSend = bodystr.size();

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


// 4 - Getter

int	ClientConnection::getFd(void) const { return _fd; }

// 5 - Reading function called from server loop epoll

ClientConnection::ReadStatus	ClientConnection::readFromFd(void)
{
	char	recvBuffer[8192];
	ssize_t	bytesRead = 0;
	size_t	totalBytesRead;

	while (true)
	{
		bytesRead = recv(_fd, recvBuffer, sizeof(recvBuffer), MSG_DONTWAIT);
		
		if (bytesRead > 0)
		{
			_httpRequest.appendToBuffer(recvBuffer, static_cast<size_t>(bytesRead));
			switch (_httpRequest.tryParseHttpRequest())
			{
				case HttpRequest::PARSING_HEADERS: 		break;	// Read again now
				case HttpRequest::PARSING_HEADERS_DONE: if (prepareResponseFromRequestHeadersComplete() == HttpResponse::READY_TO_SEND) return (READ_OK); break;
				case HttpRequest::PARSING_BODY: 		break;	// Read again now
				case HttpRequest::PARSING_BODY_DONE:	return (prepareResponseFromRequestBodyComplete());
				case HttpRequest::INVALID:				return (prepareResponseInvalidRequest(_httpRequest.getStatusCode(), READ_OK));
				default:								break;
			}	
		}
		else if (bytesRead == 0)
		{
			Console::log(Console::WARNING, "Client disconnected before request was complete");
			return (READ_CONNECTION_LOST);
		}
		else
		{
			if (_httpRequest.getBuffer().empty())
			{
				Console::log(Console::ERROR, "Reading from fd failed");
				return (prepareResponseInvalidRequest(500, READ_RECV_ERROR));
			}
			return (READ_AGAIN_LATER);
		}
	}
	return (READ_OK);
}

ClientConnection::WriteStatus		ClientConnection::sendToFd(void)
{
	TStr	responseHeaders = _httpResponse.toString();
	ssize_t bytesSent = send(_fd, responseHeaders.c_str(), responseHeaders.size(), 0);

	switch (_httpResponse.getBodyType())
	{
		case HttpResponse::NO_BODY:			return (WRITE_OK);
		case HttpResponse::FILEDESCRIPTOR:	sendResponseBodyFd(_httpResponse.getBodyFd()); break ;
		case HttpResponse::STRING:			sendResponseBodyStr(_httpResponse.getBodyStr()); break;
		default :							break;
	}
	return (WRITE_OK);
}




