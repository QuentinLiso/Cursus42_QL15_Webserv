/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_ListeningSocket.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/31 12:59:44 by qliso             #+#    #+#             */
/*   Updated: 2025/06/01 10:39:27 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef	LISTENING_SOCKET_HPP
# define LISTENING_SOCKET_HPP

#include "Includes.hpp"
#include "Console.hpp"
#include "0_Utils.hpp"

class	ListeningSocket
{
	private:
		TIPPort				_ipPort;
		TStr				_ipStr;
		struct sockaddr_in	_addr;
		int					_sockfd;
		int					_error;

		int		createSocket(void);
		int		setReuseAddr(void);
		int		bindSocket(void);
		int		listenSocket(void);

		int		error(void);

	public:
		ListeningSocket(const TIPPort& ipPort);
		virtual ~ListeningSocket(void);

		bool	validSocket(void) const;
		int		getSockFd(void) const;

		void	makeListeningSocketReady(void);
		int		closeSocket(void);
		int		acceptConnections(void);
		void	logIpClient(struct sockaddr_in*	addr);
};


#endif