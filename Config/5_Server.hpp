/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   5_Server.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:42 by qliso             #+#    #+#             */
/*   Updated: 2025/06/02 12:44:04 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	SERVER_HPP
# define SERVER_HPP

#include "Includes.hpp"
#include "0_Utils.hpp"
#include "3_Build.hpp"
#include "4_ListeningSocket.hpp"

class	Server
{
	private:
		int									_epollfd;
		int									_eventsReady;
		struct epoll_event					_eventQueue[64];

		std::vector<ListeningSocket*>		_sockets;
		std::map<int, ListeningSocket*>		_fdToSocketMap;
		std::set<int>						_clientfds;
		int									_error;
		
		int	addListeningSockets(const std::set<TIPPort>& ips);
		int registerSingleFdToEpoll(int fd);
		int	registerSocketsToEpoll(void);
		int	waitForConnections(int timeout = -1);
		int	acceptConnection(int listeningSockFd);
		void	logIpClient(struct sockaddr_in* addr, int listeningSockFd, int clientfd) const;

		int	error(const TStr& msg);
		
	public:
		Server(void);
		virtual ~Server(void);

		void	makeServerReady(const std::set<TIPPort>& ips);
		void	run(void);
};


#endif