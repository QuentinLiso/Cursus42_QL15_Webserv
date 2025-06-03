/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:09 by qliso             #+#    #+#             */
/*   Updated: 2025/06/03 20:01:34 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "6_ClientConnection.hpp"

// Client Connection

ClientConnection::ClientConnection(int fd)
        :   _fd(fd),
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
			_httpRequest.setValidRequestForTesting();
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

	static const char response[] = "HTTP/1.1 200 OK\r\n"
									"Content-Type: text/html\r\n"
									"Content-Length: 13\r\n"
									"Connection: close\r\n"
									"\r\n"
									"Hello, world!\n";
	// std::cout << "Message received : " << _httpRequest.getBuffer() << std::endl;
	_httpRequest.printRequest(std::cout);
	send(_fd, response, sizeof(response), 0);
	return (READ_OK);
}






