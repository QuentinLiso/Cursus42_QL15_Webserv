/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   5_Server.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:42 by qliso             #+#    #+#             */
/*   Updated: 2025/06/01 12:15:38 by qliso            ###   ########.fr       */
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
		static const int					_maxEvents;
		int									_epollfd;
		struct epoll_event					_events[64];
		std::vector<ListeningSocket>		_sockets;
		std::map<int, ListeningSocket*>		_fdToSocketMap;
		
		int	addListeningSockets(void);
		int	registerSocketsToEpoll(void);
		int	waitForConnections(int timeout);
		
	public:
		Server(void);
		virtual ~Server(void);

		int	makeServerReady(const std::set<TIPPort>& ips);
};


#endif