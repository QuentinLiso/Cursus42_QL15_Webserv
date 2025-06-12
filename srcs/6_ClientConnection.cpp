/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:09 by qliso             #+#    #+#             */
/*   Updated: 2025/06/12 19:11:47 by qliso            ###   ########.fr       */
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


// 3 - Handling request and response after reading
void	ClientConnection::handleCompleteRequest(void)
{
	const LocationConfig* loc = _relatedListeningSocket->findLocationConfig(_httpRequest.getHostAddress(), _httpRequest.getUriPath());
	
	// _httpRequest.printRequest(std::cout);
	_httpResponse.prepareResponse(&_httpRequest, loc);

	// _httpResponse.print();
	TStr	responseHeaders = _httpResponse.toString();
	send(_fd, responseHeaders.c_str(), responseHeaders.size(), 0);

	switch (_httpResponse.getBodyType())
	{
		case HttpResponse::FILEDESCRIPTOR:
			sendResponseBodyFd(_httpResponse.getBodyFd());
			break ;
		case HttpResponse::STRING:
			sendResponseBodyStr(_httpResponse.getBodyStr());
			break;
		default :
			break;
	}
}

void	ClientConnection::handleErrorRequest(unsigned short code)
{
	_httpResponse.prepareErrorResponse(code);
	TStr	responseHeaders = _httpResponse.toString();

	send(_fd, responseHeaders.c_str(), responseHeaders.size(), 0);
	sendResponseBodyStr(_httpResponse.getBodyStr());
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

int		ClientConnection::readFromFd(void)
{
	char	recvBuffer[1024];
	ssize_t	bytesRead = 0;
	size_t	totalBytesRead;

	while (true)
	{
		bytesRead = recv(_fd, recvBuffer, sizeof(recvBuffer), MSG_DONTWAIT);
		
		if (bytesRead > 0)
		{
			_httpRequest.appendToBuffer(recvBuffer, static_cast<size_t>(bytesRead));
			if (_httpRequest.tryParseHttpRequest())
				break ;
			if (static_cast<size_t>(bytesRead) < sizeof(recvBuffer))
				break ;
		}
		else if (bytesRead == 0)
		{
			Console::log(Console::WARNING, "Client disconnected before request was complete");
			return (CONNECTION_LOST);
		}
		else
		{
			Console::log(Console::ERROR, "Reading from fd failed");
			handleErrorRequest(500);
			return (RECV_ERROR);
		}
	}

	if (_httpRequest.getStatus() == HttpRequest::INVALID)
		handleErrorRequest(_httpRequest.getStatusCode());
	else
		handleCompleteRequest();
	return (READ_OK);
}






