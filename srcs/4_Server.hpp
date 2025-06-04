/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_Server.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:42 by qliso             #+#    #+#             */
/*   Updated: 2025/06/04 21:49:56 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	SERVER_HPP
# define SERVER_HPP

#include "Includes.hpp"
#include "0_Utils.hpp"
#include "3_Build.hpp"
#include "5_ListeningSocket.hpp"
#include "6_ClientConnection.hpp"
#define MAX_FD 1024

class	Server
{
	private:
		int						_epollfd;
		int						_eventsReady;
		struct epoll_event		_eventQueue[64];

		ListeningSocket*		_socketsfds[MAX_FD];
		ClientConnection*		_clientsfds[MAX_FD];

		int						_error;
		
		int		addListeningSockets(const std::map<TIPPort, HostToServerMap>& runtimeBuild);
		int 	registerSingleFdToEpoll(int fd);
		int		registerSocketsToEpoll(void);
		int		waitForConnections(int timeout = -1);
		int		acceptConnection(ListeningSocket* listeningSocket);
		
		void	logIpClient(struct sockaddr_in* addr, int listeningSockFd, int clientfd) const;
		int		getClientRequest(int fd);
		int		closeConnection(int fd);
		
		int	error(const TStr& msg);
		
	public:
		Server(void);
		virtual ~Server(void);

		void	makeServerReady(const Builder& builder);
		void	run(void);
};

#endif