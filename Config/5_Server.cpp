/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   5_Server.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:45 by qliso             #+#    #+#             */
/*   Updated: 2025/06/03 01:19:34 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "5_Server.hpp"

int Server::addListeningSockets(const std::set<TIPPort>& ips)
{
    _epollfd = epoll_create1(0);
    if (_epollfd == -1)
        return (error("[SERVER] : epoll create failed, server cannot be launched"));
    
    for (std::set<TIPPort>::const_iterator it = ips.begin(); it != ips.end(); it++)
    {
        ListeningSocket* listeningSocket = new ListeningSocket(*it);
        if (listeningSocket->makeListeningSocketReady())
		{
			delete	listeningSocket;
			_error = 1;
			Console::log(Console::ERROR, "Socket creation failed, FD not added to server");
			continue ;
		}
		int	fd = listeningSocket->getSockFd();
		if (fd < 0 || fd >= MAX_FD)
		{
			delete listeningSocket;
			_error = 1;
			Console::log(Console::ERROR, "Socket FD is too high, not added to server");
			continue ;
		}
		_socketsfds[fd] = listeningSocket;
    }
    return (_error);
}

int Server::registerSingleFdToEpoll(int fd)
{
    struct epoll_event  eventRegister;
    eventRegister.events = EPOLLIN;
    eventRegister.data.fd = fd;
    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, fd, &eventRegister) == -1)
    {
        std::ostringstream  oss;
        oss << "[SERVER] : Couldn't add FD " << fd << "to server epoll";
        Console::log(Console::ERROR, oss.str());
        return (1);
    }
    return (0);
}

int Server::registerSocketsToEpoll(void)
{
    for (size_t i = 0; i < MAX_FD; i++)
    {
        if (_socketsfds[i] != NULL && registerSingleFdToEpoll(i))
            _error = 1;
    }
    return (_error);
}

int Server::waitForConnections(int timeout)
{
	// int	loop = 4;
    while (true)
    {
        _eventsReady = epoll_wait(_epollfd, _eventQueue, 64, timeout);
		if (_eventsReady == -1)
		{
			Console::log(Console::ERROR, strerror(errno));
			continue ;
		}
        for (int i = 0; i < _eventsReady; i++)
        {
            int fd = _eventQueue[i].data.fd;
			if (fd < 0 || fd >= MAX_FD)
				Console::log(Console::ERROR, "Got an event inside a FD that is neither a listening socket nor a client open connection");
            else if (_socketsfds[fd] != NULL)
                acceptConnection(fd);
            else if (_clientsfds[fd] != NULL)
				getClientRequest(fd);
            else
                Console::log(Console::ERROR, "Got an event inside a FD that is neither a listening socket nor a client open connection");
        }
    }
	return (0);
}

int     Server::acceptConnection(int listeningSockFd)
{
    struct sockaddr_storage clientAddr;
    socklen_t               clientAddrLen = sizeof(clientAddr);
    int        				clientfd = accept(listeningSockFd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    
    if (clientfd < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			Console::log(Console::DEBUG, "No connection available now");
        else
		    Console::log(Console::ERROR, "Failed to accept client connection");
		return (-1);
	}
    _clientsfds[clientfd] = new ClientConnection(clientfd);
    registerSingleFdToEpoll(clientfd);
	logIpClient((struct sockaddr_in*)&clientAddr, listeningSockFd, clientfd);
    return (clientfd);
}

int		Server::getClientRequest(int fd)
{
	ClientConnection* connection = _clientsfds[fd];
	int status = connection->readFromFd();
	if (status > 0 && connection->isRequestComplete())
	{
		std::cout << "Message received : " << connection->getRequestBuffer() << std::endl;
		send(fd, "Bonjour\n", 8, 0);
		closeConnection(fd);
	}
	else if (status > 0 && !connection->isRequestComplete())
	{
		std::cout << "Message received, exceeding buffer limit : " << connection->getRequestBuffer() << std::endl;
		send(fd, "Bonjour\n", 8, 0);
		closeConnection(fd);
	}
	else if (status == 0)
	{
		Console::log(Console::WARNING, "Client disconnected before request was complete");
		closeConnection(fd);
	}
	else if (status < 0)
	{
		Console::log(Console::ERROR, "Reading from fd failed");
		closeConnection(fd);
	}
	return (0);
}

int	Server::closeConnection(int fd)
{
	if (_clientsfds[fd] != NULL)
	{
		delete _clientsfds[fd];
		if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) == -1)
			Console::log(Console::ERROR, strerror(errno));
		return (debugClose(fd));
	}
	return (0);
}

void	Server::logIpClient(struct sockaddr_in* addr, int listeningSockFd, int clientfd) const
{
	char	ipStr[INET_ADDRSTRLEN];

	if (inet_ntop(addr->sin_family, (void *)&addr->sin_addr, ipStr, sizeof(ipStr)))
    {
        std::ostringstream  oss;
        oss << "[SERVER] Accepted connection from " << ipStr 
            << "(Socket FD : " << listeningSockFd
            << ", Client FD : " << clientfd << ")";
		Console::log(Console::INFO, oss.str());
    }
}

int Server::error(const TStr& msg)
{
    Console::log(Console::ERROR, msg);
    _error = 1;
    return (_error);
}




Server::Server(void)
        :   _epollfd(-1),
            _eventsReady(-1),
			_socketsfds(),
			_clientsfds(),
            _error(0)
{}

Server::~Server(void)
{
    if (_epollfd != -1)
        debugClose(_epollfd);
    for(size_t i = 0; i < MAX_FD; i++)
	{
		if (_socketsfds[i] != NULL)
			delete _socketsfds[i];
		// if (_clientsfds[i] != NULL)
		// 	delete _clientsfds[i];
	}
}

void Server::makeServerReady(const std::set<TIPPort>& ips)
{
    if (addListeningSockets(ips) || registerSocketsToEpoll())
        throw std::runtime_error ("Server could not be built");
}

void    Server::run(void)
{
    waitForConnections();
}