/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_Server.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:45 by qliso             #+#    #+#             */
/*   Updated: 2025/06/21 17:29:14 by qliso            ###   ########.fr       */
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
			case	FdType::FD_LISTENING_SOCKET:	delete static_cast<ListeningSocket*>(context->_data); break;
			case	FdType::FD_CLIENT_CONNECTION:	delete static_cast<ClientConnection*>(context->_data); break;
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
		registerNewFdToEpoll(fd, EPOLLIN, EPOLL_CTL_ADD, listeningSocket, FdType::FD_LISTENING_SOCKET);
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
		case	FdType::FD_LISTENING_SOCKET: 	createClientConnection(static_cast<ListeningSocket*>(context->_data));	break;
		case	FdType::FD_CLIENT_CONNECTION:	handleClientConnection(context, events);								break;
		case	FdType::FD_CGI_PIPE:			handleClientConnection(context, events);								break;
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
	    Console::log(Console::ERROR, "Failed to accept client connection");
		return (-1);
	}
	logIpClient((struct sockaddr_in*)&clientAddr, listeningSocket->getSockFd(), clientfd);
	registerNewFdToEpoll(clientfd, EPOLLIN | EPOLLOUT, EPOLL_CTL_ADD, new ClientConnection(*this, clientfd, listeningSocket), FdType::FD_CLIENT_CONNECTION);
    return (clientfd);
}

void	Server::handleClientConnection(FdContext* context, uint32_t events)
{
	int	loops = 0;
	
	ClientConnection*	client = static_cast<ClientConnection*>(context->_data);
	
	client->handleEvent(events, context->_fd, context->_fdType);
	while (client->needEpollEventToProgress() == false)
		client->handleEvent(events, context->_fd, context->_fdType);
	if (client->getClientConnectionState() == ClientConnection::STATE_CLOSING_CONNECTION)
		deregisterFdFromEpoll(context->_fd, FdType::FD_CLIENT_CONNECTION);
}


// Logs
void	Server::logIpClient(struct sockaddr_in* addr, int listeningSockFd, int clientfd) const
{
	char	ipStr[INET_ADDRSTRLEN];

	if (inet_ntop(addr->sin_family, (void *)&addr->sin_addr, ipStr, sizeof(ipStr)))
    {
		std::cout << std::endl;
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
void	Server::registerNewFdToEpoll(int fd, int epollEvent, int epollCtlOperation, void* data, FdType::Type fdType)
{
	if (registerSingleFdToEpoll(fd, epollEvent, epollCtlOperation, fdType))
		return ;

	registerFdContext(fd, data, fdType);
}

int 	Server::registerSingleFdToEpoll(int fd, int epollEvent, int epollCtlOperation, FdType::Type fdType)
{
	Console::log(Console::INFO, "[SERVER] Epoll ctl called on : FD " + convToStr(fd) + " " + FdType::toString(fdType));
    struct epoll_event  eventRegister;
    eventRegister.events = epollEvent;
    eventRegister.data.fd = fd;
    if (epoll_ctl(_epollfd, epollCtlOperation, fd, &eventRegister) == -1)
    {
        std::ostringstream  oss;
        oss << "[SERVER] : Epoll CTL failed";
        Console::log(Console::ERROR, oss.str());
        return (1);
    }
    return (0);
	// Used to switch client connection to EPOLLOUT
}

void	Server::registerFdContext(int fd, void* data, FdType::Type fdType)
{
	Console::log(Console::INFO, "[SERVER] Registering to context : FD " + convToStr(fd) + " " + FdType::toString(fdType));
	FdContext*	fdContext = new FdContext();

	fdContext->_fd = fd;
	fdContext->_data = data;
	fdContext->_fdType = fdType;
	
	_fdContexts[fd] = fdContext;
}

void	Server::deregisterFdFromEpoll(int fd, FdType::Type fdType)
{
	Console::log(Console::INFO, "[SERVER] Deregistering : FD " + convToStr(fd));
	if (fd >= 0 && fd < MAX_FD)
	{
		FdContext*	context = _fdContexts[fd];
		if (context != NULL)
		{
			Console::log(Console::INFO, "[SERVER] Deregistering from context : FD " + convToStr(fd) + " " + FdType::toString(fdType));
			switch (context->_fdType)
			{
				case	FdType::FD_LISTENING_SOCKET:	delete static_cast<ListeningSocket*>(context->_data); break;
				case	FdType::FD_CLIENT_CONNECTION:	delete static_cast<ClientConnection*>(context->_data); break;
				default :						break ;
			}
			delete context;
		}
		Console::log(Console::INFO, "[SERVER] Epoll ctl DEL on : FD " + convToStr(fd) + " " + FdType::toString(fdType));
		if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) == -1)
			Console::log(Console::ERROR, strerror(errno));
		Console::log(Console::INFO, "[SERVER] Closing  : FD " + convToStr(fd) + " " + FdType::toString(fdType));
		close(fd);
	}
	// Used to close EPOLLIN/EPOLLOUT for CGI, and even client connection
}


// Switch client connection to EPOLLOUT
// Create EPOLLIN for CGI + add to fd context
// Create EPOLLOUT for CGI
// Delete EPOLLIN for CGI
// Delete EPOLLOUT for CGI
// Delete client connection

// void	Server::handleFailingEpollCtl(ClientConnection& client)
// {
// 	int	fd1 = client.getFd();
// 	epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd1, NULL);
// 	FdContext*	context1 = _fdContexts[fd1];
// 	close(fd1);
// 	if (context1) {
// 		delete context1->_data;  // Assuming this deletes the ClientConnection
// 		delete context1;
// 		_fdContexts[fd1] = NULL;
// 	}

// 	int fd2 = client.getCgiHandler().getInputPipeWrite();
// 	if (fd2 > 0) epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd2, NULL);
// 	if (fd2 > 0) close(fd2);
// 	FdContext*	context2 = _fdContexts[fd2];
// 	if (context2) {
// 			delete context2;
// 			_fdContexts[fd2] = NULL;
// 		}

// 	int fd3 = client.getCgiHandler().getOutputPipeRead();
// 	if (fd3 > 0) epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd3, NULL);
// 	if (fd3 > 0) close(fd3);
// 	FdContext*	context3 = _fdContexts[fd3];
// 	if (context3) {
// 			delete context3;
// 			_fdContexts[fd3] = NULL;
// 		}
	
// }
	

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