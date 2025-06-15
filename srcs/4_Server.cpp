/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_Server.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:45 by qliso             #+#    #+#             */
/*   Updated: 2025/06/16 00:40:47 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "4_Server.hpp"

// Constructor & destructor

Server::Server(void)
        :   _epollfd(-1),
            _eventsReady(-1),
            _error(0)
{}

Server::~Server(void)
{
    if (_epollfd != -1)
        debugClose(_epollfd);
    for(size_t i = 0; i < MAX_FD; i++)
	{
		FdContext*	context = _fdContexts[i];
		switch (context->_fdType)
		{
			case	LISTENING_SOCKET:	delete static_cast<ListeningSocket*>(context->_data); break;
			case	CLIENT_CONNECTION:	delete static_cast<ClientConnection*>(context->_data); break;
			default :					break ;
		}
		delete context;
	}
}

// Init making server ready
int	Server::createEpoll(void)
{
    _epollfd = epoll_create1(0);
	
    if (_epollfd == -1)	return (error("[SERVER] : epoll create failed, server cannot be launched"));

	return (0);
}

int Server::createListeningSockets(const std::map<TIPPort, HostToServerMap>& runtimeBuild)
{
    for (std::map<TIPPort, HostToServerMap>::const_iterator it = runtimeBuild.begin(); it != runtimeBuild.end(); it++)
    {
        ListeningSocket* listeningSocket = new ListeningSocket(it->first, it->second);
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
		registerFdContext(fd, listeningSocket, Server::LISTENING_SOCKET);	
		registerSingleFdToEpollFd(fd, EPOLLIN, EPOLL_CTL_ADD);
    }
    return (_error);
}


// Monitor server activity
void	Server::fdActivityMonitor(int eventQueueIndex)
{
	int			fd = _eventQueue[eventQueueIndex].data.fd;
	uint32_t	events = _eventQueue[eventQueueIndex].events;

	if (fd < 0 || fd >= MAX_FD || _fdContexts[fd] == NULL)
	{
		Console::log(Console::ERROR, "Got an event inside a FD that is not in the epoll wait queue");
		return ;
	}

	FdContext*	context = _fdContexts[fd];
	switch (context->_fdType)
	{
		case	LISTENING_SOCKET: createClientConnection(static_cast<ListeningSocket*>(context->_data)); break;
		case	CLIENT_CONNECTION:
			if (events & EPOLLIN)
				getClientRequest(fd);
			if (events & EPOLLOUT)
				getClientResponse(fd);
			break ;
		default	:	
			Console::log(Console::ERROR, "Got an event inside a FD that is neither a listening socket nor a client open connection");
			break ;
	}
}

int     Server::createClientConnection(ListeningSocket* listeningSocket)
{
    struct sockaddr_storage clientAddr;
    socklen_t               clientAddrLen = sizeof(clientAddr);
    int        				clientfd = accept(listeningSocket->getSockFd(), (struct sockaddr *)&clientAddr, &clientAddrLen);
    
    if (clientfd < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			Console::log(Console::DEBUG, "No connection available now");
        else
		    Console::log(Console::ERROR, "Failed to accept client connection");
		return (-1);
	}
	registerFdContext(clientfd, new ClientConnection(clientfd, listeningSocket), CLIENT_CONNECTION);
    registerSingleFdToEpollFd(clientfd, EPOLLIN, EPOLL_CTL_ADD);
	logIpClient((struct sockaddr_in*)&clientAddr, listeningSocket->getSockFd(), clientfd);
    return (clientfd);
}

int		Server::getClientRequest(int fd)
{
	int	readingResult = static_cast<ClientConnection*>(_fdContexts[fd]->_data)->readFromFd();
	switch (readingResult)
	{
		case ClientConnection::READ_CONNECTION_LOST:	closeConnection(fd); break;
		case ClientConnection::READ_OK:
		case ClientConnection::READ_RECV_ERROR:			registerSingleFdToEpollFd(fd, EPOLLOUT, EPOLL_CTL_MOD); break;
		default:										break;
	}
	return (0);
}

int		Server::getClientResponse(int fd)
{
	int writingResult = static_cast<ClientConnection*>(_fdContexts[fd]->_data)->sendToFd();

	switch (writingResult)
	{
		case ClientConnection::WRITE_OK:	closeConnection(fd);
		default:							break;
	}
	return (0);
}

int		Server::closeConnection(int fd)
{
	if (fd >= 0 && fd < MAX_FD)
	{
		FdContext*	context = _fdContexts[fd];
		switch (context->_fdType)
		{
			case	LISTENING_SOCKET:	delete static_cast<ListeningSocket*>(context->_data); break;
			case	CLIENT_CONNECTION:	delete static_cast<ClientConnection*>(context->_data); break;
			default :					break ;
		}
		delete context;
		
		if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) == -1)
			Console::log(Console::ERROR, strerror(errno));
		return (debugClose(fd));
	}
	return (0);
}


// Logs
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

int 	Server::error(const TStr& msg)
{
    Console::log(Console::ERROR, msg);
    _error = 1;
    return (_error);
}

// Handle fds
void	Server::registerFdContext(int fd, void* data, FdType fdType)
{
	FdContext*	fdContext = new FdContext();

	fdContext->_fd = fd;
	fdContext->_data = data;
	fdContext->_fdType = fdType;
	
	_fdContexts[fd] = fdContext;
}

int 	Server::registerSingleFdToEpollFd(int fd, int epollEvent, int epollCtlOperation)
{
    struct epoll_event  eventRegister;
    eventRegister.events = epollEvent;
    eventRegister.data.fd = fd;
    if (epoll_ctl(_epollfd, epollCtlOperation, fd, &eventRegister) == -1)
    {
        std::ostringstream  oss;
        oss << "[SERVER] : Couldn't add FD " << fd << "to server epoll";
        Console::log(Console::ERROR, oss.str());
        return (1);
    }
    return (0);
}

// Make server ready
void Server::makeServerReady(const Builder& builder)
{
    if (createEpoll() || createListeningSockets(builder.getRuntimeBuild()))
        throw std::runtime_error ("Server could not be built");
}

void    Server::run(int timeout)
{
    while (true)
    {
        _eventsReady = epoll_wait(_epollfd, _eventQueue, 64, timeout);
		if (_eventsReady == -1)
		{
			Console::log(Console::ERROR, strerror(errno));
			continue ;
		}
        for (int i = 0; i < _eventsReady; i++)
			fdActivityMonitor(i);
    }
}