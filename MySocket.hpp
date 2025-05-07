/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MySocket.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/30 20:00:20 by qliso             #+#    #+#             */
/*   Updated: 2025/05/04 14:10:50 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MYSOCKET_HPP
# define MYSOCKET_HPP

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <cstring>
#include <cstdio>
#include <stdexcept>


class   MySocket
{
    private:
        int     _fd;

		void	_createAndBindSocket(const std::string& port);
		void	_listenSocket(int backlog);
	
	public:
        MySocket(void);
        virtual ~MySocket(void);

		int	getFd(void) const;

		void	setSocket(const std::string& port, int backlog);
		int		acceptConnections(void);
		void	showConnectedClient(struct sockaddr_storage clientAddr) const;
};


#endif