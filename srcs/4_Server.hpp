/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_Server.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/01 12:02:42 by qliso             #+#    #+#             */
/*   Updated: 2025/06/24 16:23:52 by qliso            ###   ########.fr       */
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

class	ClientConnection;

class	Server
{
	public :
		Server(void);
		virtual ~Server(void);

	private:
		struct FdContext {
			int				_fd;
			void*			_data;
			FdType::Type	_fdType;
			time_t			_creationTime;
			time_t			_timeout;


			FdContext();
			~FdContext();
		};
		
		FdContext				_fdContexts[MAX_FD];
		int						_epollfd;
		int						_eventsReady;
		struct epoll_event		_eventQueue[64];
		bool					_running;
		int						_error;
		time_t					_lastTimeoutCheck;

		// Init
		int		createEpoll(void);
		int		createListeningSockets(const std::map<TIPPort, HostToServerMap>& runtimeBuild);

		// Monitor fd activity
		void	fdActivityMonitor(int eventQueueIndex);
		int     createClientConnection(ListeningSocket* listeningSocket);
		void	handleClientConnection(ClientConnection* clientConnection, int fd, FdType::Type fdType, uint32_t events);
		static void signalHandler(int signum);

		// Logs
		void	logIpClient(struct sockaddr_in* addr, int listeningSockFd, int clientfd) const;
		int	error(const TStr& msg);
		
	public:
		// Handle fds
		int		registerFdToEpoll(int fd, int epollEvent, int epollCtlOperation, void* data, FdType::Type fdType);
		void	deregisterFdFromEpoll(int fd);
		void	deregisterFdsInForkChild(void);

		void	monitorTimeouts(void);

		// Make server ready
		void	makeServerReady(const Builder& builder, int signalPipeReadFd, int signalPipeWriteFd);
		void	run(void);
};

#endif