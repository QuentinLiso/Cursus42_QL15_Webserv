/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_Server.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:45 by qliso             #+#    #+#             */
/*   Updated: 2025/06/24 12:16:11 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "4_Server.hpp"

// Constructor & destructor
Server::FdContext::FdContext(void)
	:	_fd(-1), 
		_data(NULL),
		_fdType(FdType::FD_UNDEFINED)
{}

Server::FdContext::~FdContext(void)
{
	if (_data != NULL)
	{
		if (_fdType == FdType::FD_CLIENT_CONNECTION)
			delete static_cast<ClientConnection*>(_data);
		else if (_fdType == FdType::FD_LISTENING_SOCKET)
			delete static_cast<ListeningSocket*>(_data);
	}
}


Server::Server(void)
        :   _epollfd(-1),
            _eventsReady(-1),
			_running(false),
            _error(0)
{}

Server::~Server(void)
{
	for (int fd = 0; fd < MAX_FD; fd++)
	{
		switch (_fdContexts[fd]._fdType)
		{
			case FdType::FD_UNDEFINED :
				break;
			
			case FdType::FD_EPOLL :				
				close(fd); 
				_fdContexts[fd]._fd = -1;
				_fdContexts[fd]._fdType = FdType::FD_UNDEFINED;	
				break;
			
			case FdType::FD_LISTENING_SOCKET :	
				close(fd);
				_fdContexts[fd]._fd = -1;
				if (_fdContexts[fd]._data != NULL) delete static_cast<ListeningSocket*>(_fdContexts[fd]._data); _fdContexts[fd]._data = NULL;
				_fdContexts[fd]._data = NULL;
				_fdContexts[fd]._fdType = FdType::FD_UNDEFINED;
				break;

			case FdType::FD_CLIENT_CONNECTION :	
				close(fd); 
				_fdContexts[fd]._fd = -1;
				if (_fdContexts[fd]._data != NULL) delete static_cast<ClientConnection*>(_fdContexts[fd]._data); _fdContexts[fd]._data = NULL;
				_fdContexts[fd]._data = NULL;
				_fdContexts[fd]._fdType = FdType::FD_UNDEFINED;
				break;

			case FdType::FD_CGI_PIPE :
				close(fd);	
				break;

			case FdType::FD_SIGNAL_PIPE :
				close(fd); 
				_fdContexts[fd]._fd = -1;
				_fdContexts[fd]._data = NULL;
				_fdContexts[fd]._fdType = FdType::FD_UNDEFINED;
				break;
			
			default :
				break;
		}
	}
	Console::log(Console::INFO, "[SERVER] Closing EPOLL : FD " + convToStr(_epollfd));
	close(_epollfd);
	_epollfd = -1;
	Console::log(Console::INFO, "\nI'm tired, time to go to sleep ! See you around :)");
}

// Init making server ready
int	Server::createEpoll(void)
{
    _epollfd = epoll_create1(0);
	
    if (_epollfd == -1)	return (error("[SERVER] : epoll create failed, server cannot be launched"));
    if (_epollfd >= MAX_FD)	return (error("[SERVER] : epoll fd too high for stored num of fds"));

	Console::log(Console::INFO, "[SERVER] Registering to context : FD " + convToStr(_epollfd) + " " + FdType::toString(FdType::FD_EPOLL));
	_fdContexts[_epollfd]._fd = _epollfd;
	_fdContexts[_epollfd]._data = NULL;
	_fdContexts[_epollfd]._fdType = FdType::FD_EPOLL;
	Console::log(Console::DEBUG, "[SERVER] Epoll fd opened on FD : " + convToStr(_epollfd));
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
		registerFdToEpoll(fd, EPOLLIN, EPOLL_CTL_ADD, listeningSocket, FdType::FD_LISTENING_SOCKET);
    }
    return (_error);
}


// Monitor server activity
void	Server::fdActivityMonitor(int eventQueueIndex)
{
	int			fd = _eventQueue[eventQueueIndex].data.fd;
	uint32_t	events = _eventQueue[eventQueueIndex].events;

	if (fd < 0 || fd >= MAX_FD)
	{
		// Console::log(Console::ERROR, "Got an event inside a FD that is not in the epoll wait queue");
		return ;
	}

	switch (_fdContexts[fd]._fdType)
	{
		case	FdType::FD_LISTENING_SOCKET: 	createClientConnection(static_cast<ListeningSocket*>(_fdContexts[fd]._data));			break;
		case	FdType::FD_CLIENT_CONNECTION:	handleClientConnection(static_cast<ClientConnection*>(_fdContexts[fd]._data), fd, _fdContexts[fd]._fdType, events);	break;
		case	FdType::FD_CGI_PIPE:			handleClientConnection(static_cast<ClientConnection*>(_fdContexts[fd]._data), fd, _fdContexts[fd]._fdType, events);	break;
		case	FdType::FD_SIGNAL_PIPE:			_running = false;	break ;
		default	:	
			Console::log(Console::ERROR, "Got an event inside a FD that is not a socket, client connection or cgi pipe");
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
	registerFdToEpoll(clientfd, EPOLLIN | EPOLLOUT | EPOLLHUP, EPOLL_CTL_ADD, new ClientConnection(*this, clientfd, listeningSocket), FdType::FD_CLIENT_CONNECTION);
    return (clientfd);
}

void	Server::handleClientConnection(ClientConnection* clientConnection, int fd, FdType::Type fdType, uint32_t events)
{
	clientConnection->handleEvent(events, fdType);
	while (clientConnection->needEpollEventToProgress() == false)
		clientConnection->handleEvent(events, fdType);
	if (clientConnection->getClientConnectionState() == ClientConnection::STATE_CLOSING_CONNECTION)
		deregisterFdFromEpoll(fd);
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
int	Server::registerFdToEpoll(int fd, int epollEvent, int epollCtlOperation, void* data, FdType::Type fdType)
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

	Console::log(Console::INFO, "[SERVER] Registering to context : FD " + convToStr(fd) + " " + FdType::toString(fdType));
	if (fd >= 0 && fd < MAX_FD)
	{
		_fdContexts[fd]._fd = fd;
		_fdContexts[fd]._data = data;
		_fdContexts[fd]._fdType = fdType;
	}
	return (0);
}

void	Server::deregisterFdFromEpoll(int fd)
{
	Console::log(Console::INFO, "[SERVER] Epoll ctl DEL on : FD " + convToStr(fd) + " " + FdType::toString(_fdContexts[fd]._fdType));
	if (epoll_ctl(_epollfd, EPOLL_CTL_DEL, fd, NULL) == -1)
		Console::log(Console::ERROR, strerror(errno));

	if (fd >= 0 && fd < MAX_FD)
	{	
		Console::log(Console::INFO, "[SERVER] Closing  : FD " + convToStr(fd) + " " + FdType::toString(_fdContexts[fd]._fdType));
		close(_fdContexts[fd]._fd);
		_fdContexts[fd]._fd = -1;

		if (_fdContexts[fd]._data != NULL)
		{
			switch (_fdContexts[fd]._fdType)
			{	
				case FdType::FD_LISTENING_SOCKET :	delete static_cast<ListeningSocket*>(_fdContexts[fd]._data);	break ;
				case FdType::FD_CLIENT_CONNECTION :	delete static_cast<ClientConnection*>(_fdContexts[fd]._data);	break ;
				default :	break ;
			}
			_fdContexts[fd]._data = NULL;
		}
		
		_fdContexts[fd]._fdType = FdType::FD_UNDEFINED;
	}
}

void	Server::deregisterFdsInForkChild(void)
{
	for (int fd = 0; fd < MAX_FD; fd++)
	{
		switch (_fdContexts[fd]._fdType)
		{
			case FdType::FD_UNDEFINED :
				break;
			
			case FdType::FD_EPOLL :				
				close(fd); 
				_fdContexts[fd]._fd = -1;
				_fdContexts[fd]._fdType = FdType::FD_UNDEFINED;	
				break;
			
			case FdType::FD_LISTENING_SOCKET :	
				close(fd);
				_fdContexts[fd]._fd = -1;
				// delete static_cast<ListeningSocket*>(_fdContexts[fd]._data);
				_fdContexts[fd]._data = NULL;
				_fdContexts[fd]._fdType = FdType::FD_UNDEFINED;
				break;

			case FdType::FD_CLIENT_CONNECTION :	
				close(fd); 
				_fdContexts[fd]._fd = -1;
				// delete static_cast<ClientConnection*>(_fdContexts[fd]._data);
				_fdContexts[fd]._data = NULL;
				_fdContexts[fd]._fdType = FdType::FD_UNDEFINED;
				break;

			case FdType::FD_CGI_PIPE :			
				break;

			case FdType::FD_SIGNAL_PIPE :
				close(fd); 
				_fdContexts[fd]._fd = -1;
				_fdContexts[fd]._data = NULL;
				_fdContexts[fd]._fdType = FdType::FD_UNDEFINED;
				break;
			
			default :
				break;
		}
	}
}


// Make server ready
void Server::makeServerReady(const Builder& builder, int signalPipeReadFd, int signalPipeWriteFd)
{
	(void)signalPipeWriteFd;
	if (createEpoll() || createListeningSockets(builder.getRuntimeBuild()))
        throw std::runtime_error ("Server could not be built");
	
	if (signalPipeReadFd == -1)
		return ;
	Console::log(Console::DEBUG, "[SERVER] Adding signal pipe read fd to epoll " + convToStr(signalPipeReadFd));
	registerFdToEpoll(signalPipeReadFd, EPOLLIN, EPOLL_CTL_ADD, NULL, FdType::FD_SIGNAL_PIPE);
	if (signalPipeWriteFd >= 0 && signalPipeWriteFd < MAX_FD)
	{
		_fdContexts[signalPipeWriteFd]._fd = signalPipeWriteFd;
		_fdContexts[signalPipeWriteFd]._data = NULL;
		_fdContexts[signalPipeWriteFd]._fdType = FdType::FD_SIGNAL_PIPE;
	}
	_running = true;
}


void    Server::run(int timeout)
{
    while (_running)
    {
        _eventsReady = epoll_wait(_epollfd, _eventQueue, 64, timeout);
		if (_eventsReady == -1)
		{
			Console::log(Console::ERROR, strerror(errno));
			continue ;
		}
        for (int i = 0; i < _eventsReady; i++)
		{
			fdActivityMonitor(i);
		}
    }
}