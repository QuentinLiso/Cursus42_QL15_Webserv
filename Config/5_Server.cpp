/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   5_Server.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:45 by qliso             #+#    #+#             */
/*   Updated: 2025/06/02 12:40:36 by qliso            ###   ########.fr       */
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
            _error = 1;
        _sockets.push_back(listeningSocket);
        _fdToSocketMap[listeningSocket->getSockFd()] = listeningSocket;
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
    for (std::map<int, ListeningSocket*>::const_iterator it = _fdToSocketMap.begin(); it != _fdToSocketMap.end(); it++)
    {
        if (registerSingleFdToEpoll(it->first))
            _error = 1;
    }
    return (_error);
}

int Server::waitForConnections(int timeout)
{
    while (true)
    {
        _eventsReady = epoll_wait(_epollfd, _eventQueue, 64, timeout);
        for (int i = 0; i < _eventsReady; i++)
        {
            int fd = _eventQueue[i].data.fd;
            if (_fdToSocketMap.count(fd))
                acceptConnection(fd);
            else if (_clientfds.count(fd))
                Console::log(Console::INFO, "Got a message from client :)");
            else
                Console::log(Console::ERROR, "Got an event inside a FD that is neither a listening socket nor a client open connection");
        }
    }
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
    _clientfds.insert(clientfd);
    registerSingleFdToEpoll(clientfd);
	logIpClient((struct sockaddr_in*)&clientAddr, listeningSockFd, clientfd);
    return (clientfd);
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
            _sockets(),
            _fdToSocketMap(),
            _clientfds(),
            _error(0)
{}

Server::~Server(void)
{
    if (_epollfd != -1)
        close (_epollfd);
    for(size_t i = 0; i < _sockets.size(); i++)
        delete _sockets[i]; 
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