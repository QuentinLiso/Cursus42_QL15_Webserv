/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_ListeningSocket.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/31 12:59:45 by qliso             #+#    #+#             */
/*   Updated: 2025/06/04 18:22:52 by qliso            ###   ########.fr       */
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

ListeningSocket::ListeningSocket(const TIPPort& ipPort, const HostToServerMap& hostToServerMap)
		:	_ipPort(ipPort), 
			_ipStr(ipHostByteOrderToStr(ipPort.first)), 
			_hostToServerMap(hostToServerMap),
			_defaultServerConfig(hostToServerMap.begin()->second),
			_sockfd(-1), 
			_error(0)
{
	std::memset(&_addr, 0, sizeof(_addr));
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_ipPort.second);
	_addr.sin_addr.s_addr = htonl(_ipPort.first);
}

ListeningSocket::~ListeningSocket(void) { closeSocket(); }

int	ListeningSocket::makeListeningSocketReady(void)
{
	if (createSocket() || setReuseAddr() || bindSocket() || listenSocket())
		return (1);
	std::ostringstream	oss;
	oss << "[SERVER] Listening on " << _ipStr << ":" << _ipPort.second << " (Socket FD : " << _sockfd << ")";
	Console::log(Console::INFO, oss.str());
	return (0);
}

const 	HostToServerMap& ListeningSocket::getHostToServerMap(void) const { return _hostToServerMap; }
const	ServerConfig*	ListeningSocket::getDefaultServerConfig(void) const { return _defaultServerConfig; }
int		ListeningSocket::getSockFd(void) const { return _sockfd; }
bool	ListeningSocket::getErrorSocket(void) const { return _error == 0; }

int	ListeningSocket::closeSocket(void)
{ 
	if (_sockfd != -1)
		debugClose(_sockfd);
	_sockfd = -1;
	return (0);
}

const ServerConfig*	ListeningSocket::findServerConfig(const TStr& host) const
{
	HostToServerMap::const_iterator	it = _hostToServerMap.find(host);
	if (it != _hostToServerMap.end())
		return (it->second);
	return (_defaultServerConfig);
}

const LocationConfig*	ListeningSocket::findLocationConfig(const TStr& host, const TStr& requestedUri) const
{
	return (findServerConfig(host)->findLocation(requestedUri));
}


TStr	ListeningSocket::putInfoToStr(void) const
{
	std::ostringstream	oss;
	oss << "Socket FD : " << _sockfd << " attributed to IP" << _ipStr << ":" << _ipPort.second;
	return (oss.str());
}

std::ostream&	ListeningSocket::printHostToServerMap(std::ostream& o) const
{
	o << "************ SERVER CONFIG FOR " << _ipStr << ":" << _ipPort.second << " (Socket FD : " << _sockfd << ")" << std::endl;
	for (HostToServerMap::const_iterator it = _hostToServerMap.begin(); it != _hostToServerMap.end(); it++)
	{
		o << "Server name : " << it->first << std::endl;
		it->second->print(o, 0);
	}
	return (o);
}