/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_Server.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:42 by qliso             #+#    #+#             */
/*   Updated: 2025/06/16 00:50:06 by qliso            ###   ########.fr       */
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
	public :
		enum	FdType
		{
			LISTENING_SOCKET,
			CLIENT_CONNECTION,
			CGI_PIPE
		};
		
		Server(void);
		virtual ~Server(void);

	private:
		struct FdContext {
			int		_fd;
			void*	_data;
			FdType	_fdType;
		};
		
		FdContext*				_fdContexts[MAX_FD];
		int						_epollfd;
		int						_eventsReady;
		struct epoll_event		_eventQueue[64];
		int						_error;

		// Init
		int		createEpoll(void);
		int		createListeningSockets(const std::map<TIPPort, HostToServerMap>& runtimeBuild);

		// Monitor fd activity
		void	fdActivityMonitor(int eventQueueIndex);
		int     createClientConnection(ListeningSocket* ListeningSocket);
		int		getClientRequest(int fd);
		int		getClientResponse(int fd);
		int		closeConnection(int fd);
		
		// Logs
		void	logIpClient(struct sockaddr_in* addr, int listeningSockFd, int clientfd) const;
		int	error(const TStr& msg);
		
	public:
		// Handle fds
		void	registerFdContext(int fd, void* data, FdType fdType);
		int 	registerSingleFdToEpollFd(int fd, int epollEvent, int epollCtlOperation);

		// Make server ready
		void	makeServerReady(const Builder& builder);
		void	run(int timeout = -1);
};

#endif