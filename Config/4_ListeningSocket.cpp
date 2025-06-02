/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_ListeningSocket.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/31 12:59:45 by qliso             #+#    #+#             */
/*   Updated: 2025/06/01 10:32:17 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "4_ListeningSocket.hpp"

int	ListeningSocket::createSocket(void)
{
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd == -1)
		return (error());
	return (0);
}

int	ListeningSocket::setReuseAddr(void)
{
	int	opt = 1;
	if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		closeSocket();
		return (error());
	}
	return (0);
}

int	ListeningSocket::bindSocket(void)
{
	if (bind(_sockfd, (sockaddr*)&_addr, sizeof(_addr)) == -1)
	{
		closeSocket();
		return (error());
	}
	return (0);
}

int	ListeningSocket::listenSocket(void)
{
	if (listen(_sockfd, SOMAXCONN) == -1)
	{
		closeSocket();
		return (error());
	}
	return (0);
}

int	ListeningSocket::error(void)
{
	std::ostringstream	oss;
	oss << "Socket setup failed for IP:PORT '" << _ipStr << ":" << _ipPort.second
		<< "' - ERROR : '" << strerror(errno) << "'";
	Console::log(Console::ERROR, oss.str());
	_error = 1;
	_sockfd = -1;
	return (1);
}

ListeningSocket::ListeningSocket(const TIPPort& ipPort)
		:	_ipPort(ipPort), _ipStr(ipHostByteOrderToStr(ipPort.first)), _sockfd(-1), _error(0)
{
	std::memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_ipPort.second);
	_addr.sin_addr.s_addr = htonl(_ipPort.first);
}

ListeningSocket::~ListeningSocket(void) { closeSocket(); }

void	ListeningSocket::makeListeningSocketReady(void)
{
	if (createSocket() || setReuseAddr() || bindSocket() || listenSocket())
		throw std::runtime_error("Socket creation failed");
	std::ostringstream	oss;
	oss << "[SERVER] Listening on " << _ipStr << ":" << _ipPort.second;
	Console::log(Console::INFO, oss.str());
}

bool	ListeningSocket::validSocket(void) const { return _error == 0; }
int		ListeningSocket::getSockFd(void) const { return _sockfd; }

int	ListeningSocket::closeSocket(void)
{ 
	if (_sockfd != -1)
		close(_sockfd);
	_sockfd = -1;
	return (0);
}

int     ListeningSocket::acceptConnections(void)
{
    struct sockaddr_storage clientAddr;
    socklen_t               clientAddrLen = sizeof(clientAddr);
    int        				clientfd = accept(_sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    
    if (clientfd < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
		{
			Console::log(Console::DEBUG, "No connection available now");
			return (-1);
		}
		throw std::runtime_error("Failed to accept connection");
	}
	logIpClient((struct sockaddr_in*)&clientAddr);
    return (clientfd);
}


void	ListeningSocket::logIpClient(struct sockaddr_in* addr)
{
	char	ipStr[INET_ADDRSTRLEN];

	if (inet_ntop(addr->sin_family, (void *)&addr->sin_addr, ipStr, sizeof(ipStr)))
		Console::log(Console::INFO, "[SERVER] Accepted connection from " + _ipStr);
}