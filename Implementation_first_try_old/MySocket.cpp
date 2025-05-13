/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MySocket.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 20:08:08 by qliso             #+#    #+#             */
/*   Updated: 2025/05/08 09:07:16 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MySocket.hpp"

/**
 * @brief   Creates a listening socket fd and binds it to the host addrinfo struct and port
 * 
 * 1 - Configure hints to specify the type of socket we want getaddrinfo to return
 * 
 * 2 - Get addrinfo into ai and check the return value
 * 
 * 3 - Loop through the returned addrinfo list to find a valid socket setup (socket + bind)
 * 
 * 3.A - Create a socket file descriptor using the current addrinfo entry's parameters
 * 
 * 3.B - Allow reuse of local addresses (e.g. to restart server without "address already in use" error)
 * 
 * 3.C - Attempt to bind the socket to the specified address and port
 * 
 * 4 - Free the addrinfo struct
 * 
 * 5 - Check if we reached the end of the list without successfully binding
 * @param port Lol
 */
void    MySocket::_createAndBindSocket(const std::string& port)
{
    struct addrinfo hints, *ai, *p;
    int             status;
    int             yes = 1;

    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;    
    status = getaddrinfo(NULL, port.c_str(), &hints, &ai);
    if (status < 0)
        throw std::runtime_error(std::string("getaddrinfo: ") + gai_strerror(status));
    for (p = ai; p != NULL; p = p->ai_next)
    {      
        _fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (_fd < 0)
		{
			perror("socket() failed");
			continue ;
		}   
        if (setsockopt(_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) < 0)
            throw std::runtime_error("setsockopt() failed");
        if (bind(_fd, p->ai_addr, p->ai_addrlen) == 0)
            break ;
        close(_fd);
    }
    freeaddrinfo(ai);
    if (p == NULL)
        throw std::runtime_error("failed to bind socket");
}




void    MySocket::_listenSocket(int backlog)
{
    if (listen(_fd, backlog) < 0)
        throw std::runtime_error("listen() failed");
}

void    MySocket::setSocket(const std::string& port, int backlog)
{
    _createAndBindSocket(port);
    _listenSocket(backlog);
}


MySocket::MySocket(void) : _fd(-1) 
{
    
}

MySocket::~MySocket(void)
{
    if (_fd >= 0)
        close(_fd);
}

int     MySocket::getFd(void) const
{
    return (_fd);
}

int     MySocket::acceptConnections(void)
{
    struct sockaddr_storage clientAddr;
    socklen_t               clientAddrLen = sizeof(clientAddr);
    int        				clientFd = accept(_fd, (struct sockaddr *)&clientAddr, &clientAddrLen);
    
    if (clientFd < 0)
	{
		throw std::runtime_error("Failed to accept connection");
	}
	showConnectedClient(clientAddr);
    return (clientFd);
}

void	MySocket::showConnectedClient(struct sockaddr_storage clientAddr) const
{
	char	ipAddrStr[INET6_ADDRSTRLEN];
	void	*addr;

	if (clientAddr.ss_family == AF_INET)
		addr = &(((struct sockaddr_in *)&clientAddr)->sin_addr);
	else
		addr = &(((struct sockaddr_in6 *)&clientAddr)->sin6_addr);
	
	inet_ntop(clientAddr.ss_family, addr, ipAddrStr, sizeof(ipAddrStr));
	std::cout << "Accepted connection from " << ipAddrStr << std::endl;
}

