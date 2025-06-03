/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:09 by qliso             #+#    #+#             */
/*   Updated: 2025/06/03 01:31:23 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "6_ClientConnection.hpp"


ClientConnection::ClientConnection(int fd)
        :   _fd(fd),
			_maxBytes(1024),
            _recvBuffer(),
			_requestComplete(false)
{}

ClientConnection::~ClientConnection(void) {}

int	ClientConnection::getFd(void) const { return _fd; }
const TStr&	ClientConnection::getRequestBuffer(void) const { return _recvBuffer; }
bool	ClientConnection::isRequestComplete(void) const { return _requestComplete; }

int		ClientConnection::readFromFd(void)
{
	char	buffer[1024];
	ssize_t	bytesRead = 0;
	
	size_t	remainingBytes = _maxBytes;

	while (remainingBytes > 0)
	{
		bytesRead = recv(_fd, buffer, std::min(remainingBytes, sizeof(buffer)), MSG_DONTWAIT);
		if (bytesRead > 0)
		{
			_recvBuffer.append(buffer, bytesRead);
			remainingBytes -= bytesRead;
			if (static_cast<size_t>(bytesRead) < sizeof(buffer))
				break ;
		}
		else if (bytesRead == 0)
			return (0);
		else if (errno == EAGAIN || errno == EWOULDBLOCK)
			break;
		else
			return (-1);
	}
	if (_recvBuffer.find("\n") != std::string::npos)
		_requestComplete = true;
	return (1);
}












