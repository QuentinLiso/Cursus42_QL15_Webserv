/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MyServer.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/01 13:02:42 by qliso             #+#    #+#             */
/*   Updated: 2025/05/04 22:21:14 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MyServer.hpp"

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

    ssize_t	bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytesRead == 0)
	{
		std::cout << "Client disconnected on fd " << fd << std::endl;
		close(fd);
		_fds.erase(_fds.begin() + index);
        _clients.erase(fd);
	}
	else if (bytesRead < 0)
	{
		std::cerr << "recv failed" << std::endl;
		close(fd);
		_fds.erase(_fds.begin() + index);
        _clients.erase(fd);
	}
	else
	{
		buffer[bytesRead] = '\0';
        std::map<int, MyClientConnection>::iterator   it = _clients.find(fd);
        if (it != _clients.end())
        {
            std::cout << "coucou" << std::endl;
            MyClientConnection& client = it->second;
            client.appendToBuffer(buffer, bytesRead);
            client.checkRequestCompletion();
            if (client.isRequestComplete())
            {
                std::cout << "Received from client: " << client.getRequestBuffer() << std::endl;
            }
        }
        else
        {
            std::cerr << "Client fd " << fd << " was not found in server clients list." << std::endl;
        }
	}
}

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