/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_tcpServer_linux.cpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 14:03:42 by qliso             #+#    #+#             */
/*   Updated: 2025/04/28 17:17:31 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "http_tcpServer_linux.hpp"

# include <iostream>
# include <sstream>
# include <unistd.h>

namespace
{
    const int   BUFFER_SIZE = 30720;
    
    void    log(const std::string &message)
    {
        std::cout << message << std::endl;
    }
    
    void    exitWithError(const std::string &errMessage)
    {
        log("ERROR: " + errMessage);
        exit (1);
    }    
}

namespace http
{
    TcpServer::TcpServer(std::string ip_address, int port) :
        m_ip_address(ip_address),
        m_port(port),
        m_socket(),
        m_new_socket(),
        m_incomingMessage(),
        m_socketAddress(),
        m_socketAddress_len(sizeof(m_socketAddress)),
        m_serverMessage(buildResponse())
    {
        m_socketAddress.sin_family = AF_INET;
        m_socketAddress.sin_port = htons(m_port);
        m_socketAddress.sin_addr.s_addr = inet_addr(m_ip_address.c_str());
        if (startServer() != 0)
        {
            std::ostringstream ss;
            ss << "Failed to start server with PORT : " << ntohs(m_socketAddress.sin_port);
            log(ss.str());
        }
    }

    TcpServer::~TcpServer(void)
    {
        closeServer();
    }

    int TcpServer::startServer(void)
    {
        m_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (m_socket < 0)
        {
            exitWithError("Cannot create socket");
            return (1);
        }
        if (bind(m_socket, (sockaddr *)&m_socketAddress, m_socketAddress_len) < 0)
        {
            exitWithError("Cannot connect socket to address");
            return (1);
        }
        return (0);
    }

    void    TcpServer::closeServer(void)
    {
        close(m_socket);
        close(m_new_socket);
        exit(0);        
    }

    void    TcpServer::startListen(void)
    {
        if (listen(m_socket, 20) < 0)
            exitWithError("Socket listen failed");

        std::ostringstream ss;
        ss << "\n*** Listening on ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << " PORT: " << ntohs(m_socketAddress.sin_port) << " ***\n\n";
        log(ss.str());

        int bytesReceived;

        while (1)
        {
            log("====== Waiting for a new connection ======\n\n\n");
            acceptConnection(m_new_socket);

            char    buffer[BUFFER_SIZE] = {0};
            bytesReceived = read(m_socket, buffer, BUFFER_SIZE);
            std::cout << bytesReceived << std::endl;
            if (bytesReceived < 0)
                exitWithError("Failed to read bytes from client socket connection");
            
            std::ostringstream ss;
            ss << "------ Received request from client ------\n\n";
            log(ss.str());
            sendResponse();
            close(m_new_socket);
        }
    }

    void    TcpServer::acceptConnection(int &new_socket)
    {
        new_socket = accept(m_socket, (sockaddr *)&m_socketAddress, &m_socketAddress_len);
        if (new_socket < 0)
        {
            std::ostringstream ss;
            ss << "Server failed to accept incoming connection from ADDRESS: " << inet_ntoa(m_socketAddress.sin_addr) << "; PORT: " << ntohs(m_socketAddress.sin_port);
            exitWithError(ss.str());
        }
    }

    std::string    TcpServer::buildResponse(void)
    {
        std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
        std::ostringstream ss;
        ss << "HTTP/1.1 200 OK\nContent-type: text/html\nContent-Length: " << htmlFile.size() << "\n\n" << htmlFile;

        return (ss.str());
    }

    void    TcpServer::sendResponse(void)
    {
        ssize_t    bytesSent;
    
        bytesSent = write(m_new_socket, m_serverMessage.c_str(), m_serverMessage.size());

        if ((size_t)bytesSent == m_serverMessage.size())
        {
            log("------ Server Response sent to client ------\n\n");
        }
        else
        {
            log("Error sending response to client");
        }
    }
}

