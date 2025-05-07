/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   http_tcpServer_linux.hpp                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/28 14:02:51 by qliso             #+#    #+#             */
/*   Updated: 2025/04/28 14:41:18 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef HTTP_TCPSERVER_LINUX_HPP
# define HTTP_TCPSERVER_LINUX_HPP

# include <cstdio>
# include <cstdlib>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <string>

namespace http
{
    class   TcpServer
    {
        public:
            TcpServer(std::string ip_address, int port);
            virtual ~TcpServer(void);
            void    startListen(void);

        private:
            std::string m_ip_address;
            int m_port;
            int m_socket;
            int m_new_socket;
            long    m_incomingMessage;
            struct sockaddr_in m_socketAddress;
            unsigned int    m_socketAddress_len;
            std::string m_serverMessage;


            int startServer(void);
            void closeServer(void);
            void    acceptConnection(int &new_socket);
            std::string buildResponse(void);
            void    sendResponse(void);
    }; 
} // namespace http



#endif