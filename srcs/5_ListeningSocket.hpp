/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   5_ListeningSocket.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/31 12:59:44 by qliso             #+#    #+#             */
/*   Updated: 2025/06/04 21:49:17 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef	LISTENING_SOCKET_HPP
# define LISTENING_SOCKET_HPP

#include "Includes.hpp"
#include "Console.hpp"
#include "0_Utils.hpp"
#include "3_Build.hpp"

class	ListeningSocket
{
	private:
		TIPPort					_ipPort;
		TStr					_ipStr;
		const HostToServerMap&	_hostToServerMap;
		const ServerConfig*		_defaultServerConfig;
		struct sockaddr_in		_addr;
		int						_sockfd;
		int						_error;

		int		createSocket(void);
		int		setReuseAddr(void);
		int		bindSocket(void);
		int		listenSocket(void);

		int		error(void);

	public:
		ListeningSocket(const TIPPort& ipPort, const HostToServerMap& hostToServerMap);
		virtual ~ListeningSocket(void);

		// Getters
		const 	HostToServerMap& getHostToServerMap(void) const;
		const	ServerConfig*	getDefaultServerConfig(void) const;
		int		getSockFd(void) const;
		bool	getErrorSocket(void) const;

		// Socket setup logic
		int		makeListeningSocketReady(void);
		int		closeSocket(void);
		
		// Routing helpers
		const ServerConfig*		findServerConfig(const TStr& host) const;
		const LocationConfig*	findLocationConfig(const TStr& host, const TStr& requestedUri) const;

		TStr	putInfoToStr(void) const;

		std::ostream&	printHostToServerMap(std::ostream& o) const;
};


#endif