/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MyServer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 13:02:42 by qliso             #+#    #+#             */
/*   Updated: 2025/05/13 10:51:25 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MyServer.hpp"


/*********************
    PRIVATE FUNCTIONS
**********************/


void    MyServer::_init(void)
{
    _listeningSocket.setSocket(_ports[0], 10);

    struct pollfd   pfd;
    pfd.fd = _listeningSocket.getFd();
    pfd.events = POLLIN;
    pfd.revents = 0;
    
    _fds.push_back(pfd);
}

void    MyServer::_mainLoop(void)
{
    while (1)
    {
        int     pollCount = poll(_fds.data(), _fds.size(), -1);
        if (pollCount < 0)
            throw std::runtime_error("poll() failed");
            
        for (size_t i = 0; i < _fds.size(); i++)
        {
            if (_fds[i].revents & POLLIN)
            {
                if (_fds[i].fd == _listeningSocket.getFd())
                    _handleNewConnection();
                else
                    _handleClientActivity(i);
            }
			else if (_fds[i].revents)
			{
				std::cerr << "Unhandled poll event on fd" << _fds[i].fd << std::endl;
			}
        }
    }
}

void    MyServer::_handleNewConnection(void)
{
    int clientFd = _listeningSocket.acceptConnections();

    struct pollfd   pfd;
    pfd.fd = clientFd;
    pfd.events = POLLIN;
    pfd.revents = 0;

    _fds.push_back(pfd);
	_clients.insert(std::make_pair(clientFd, MyClientConnection(clientFd)));
	std::cout << "Client connected on fd " << clientFd << std::endl;
}

void    MyServer::_handleClientActivity(size_t index)
{
    char    buffer[1024];
    int     fd = _fds[index].fd;
    ssize_t bytesRead = 1;

    while (bytesRead > 0)
    {
        bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
        if (bytesRead == 0)
        {
            _handleNoByteRead(fd, index, "Client disconnected");
        }
        else if (bytesRead < 0)
        {
            _handleNoByteRead(fd, index, "recv failed");
        }
        else
        {
            buffer[bytesRead] = '\0';
            _handleBytesRead(fd, buffer, bytesRead);
        }
    }
}

void    MyServer::_handleNoByteRead(int fd, size_t index, const std::string& msg)
{
    std::cout << "FD " << fd << ": " << msg << std::endl;
    close(fd);
    _fds.erase(_fds.begin() + index);
    _clients.erase(fd);
}

int    MyServer::_handleBytesRead(int fd, char buffer[], ssize_t bytesRead)
{
    std::map<int, MyClientConnection>::iterator   it = _clients.find(fd);
    if (it != _clients.end())
    {
        MyClientConnection& client = it->second;
        client.appendToBuffer(buffer, bytesRead);
        client.checkRequestCompletion();
        // if (client.isRequestComplete())
        // {
            std::cout << "Received from client: " << client.getRequestBuffer() << std::endl;
            client.clearBuffer();
            return (0);
        // }
    }
    else
    {
        std::cerr << "Client fd " << fd << " was not found in server clients list." << std::endl;
    }
    return (1);
}


/*********************
    PUBLIC FUNCTIONS
**********************/

MyServer::MyServer(const std::vector<std::string>& ports) : _ports(ports)
{
    _init();
}

MyServer::~MyServer(void)
{
    for (size_t i = 0; i < _fds.size(); i++)
        close(_fds[i].fd);
}

void    MyServer::runServer(void)
{
    std::cout << "Server running and waiting for connections..." << std::endl;
    _mainLoop();
}