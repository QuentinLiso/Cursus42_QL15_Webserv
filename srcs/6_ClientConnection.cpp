/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:09 by qliso             #+#    #+#             */
/*   Updated: 2025/06/07 12:09:23 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "6_ClientConnection.hpp"

// Client Connection

void	ClientConnection::handleCompleteRequest(void)
{
	const LocationConfig* loc = _relatedListeningSocket->findLocationConfig(_httpRequest.getHost(), _httpRequest.getUri());
	
	_httpRequest.printRequest(std::cout);
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


ClientConnection::ClientConnection(int fd, const ListeningSocket* relatedListeningSocket)
        :   _fd(fd),
			_relatedListeningSocket(relatedListeningSocket),
			_httpRequest()
{}

ClientConnection::~ClientConnection(void) {}

int	ClientConnection::getFd(void) const { return _fd; }

int		ClientConnection::readFromFd(void)
{
	char	buffer[1024];
	ssize_t	bytesRead = 0;
	size_t	totalBytesRead;

	while (true)
	{
		bytesRead = recv(_fd, buffer, sizeof(buffer), MSG_DONTWAIT);
		
		if (bytesRead > 0)
		{
			totalBytesRead = _httpRequest.getBuffer().size() + static_cast<size_t>(bytesRead);
			if (totalBytesRead >= HttpRequest::maxBytes)
			{
				Console::log(Console::WARNING, "Request sent by client exceeded limit");
				return (REQUEST_TOO_LONG);
			}
			_httpRequest.appendToBuffer(buffer, static_cast<size_t>(bytesRead));
			if (_httpRequest.setValidRequestForTesting())
				break ;
			if (static_cast<size_t>(bytesRead) < sizeof(buffer))
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
			return (RECV_ERROR);
		}
	}

	handleCompleteRequest();
	return (READ_OK);
}






