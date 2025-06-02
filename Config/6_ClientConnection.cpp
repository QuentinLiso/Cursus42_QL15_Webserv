/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:09 by qliso             #+#    #+#             */
/*   Updated: 2025/06/02 19:35:40 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "6_ClientConnection.hpp"


ClientConnection::ClientConnection(int fd)
        :   _fd(fd),
			_maxBytes(512),
			_totalBytesRead(0),
            _recvBuffer(),
			_requestComplete(false)
{}

ClientConnection::~ClientConnection(void) {}

int	ClientConnection::getFd(void) const { return _fd; }
const TStr&	ClientConnection::getRequestBuffer(void) const { return _recvBuffer; }
bool	ClientConnection::isRequestComplete(void) const { return _requestComplete; }

int		ClientConnection::readFromFd(void)
{
	char	buffer[64];
	ssize_t	bytesRead = recv(_fd, buffer, sizeof(buffer), 0);

	if (bytesRead > 0)
	{
		_totalBytesRead += bytesRead;
		if (_totalBytesRead > _maxBytes)
		{
			_recvBuffer.append(buffer, bytesRead - (_totalBytesRead - _maxBytes));
			_requestComplete = true;
		}
		else
			_recvBuffer.append(buffer, bytesRead);
		if (_recvBuffer.find("\n") != std::string::npos)
			_requestComplete = true;
	}
	return (bytesRead);
}