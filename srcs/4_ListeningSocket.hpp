/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   4_ListeningSocket.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/31 12:59:44 by qliso             #+#    #+#             */
/*   Updated: 2025/06/02 11:11:22 by qliso            ###   ########.fr       */
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

		int		makeListeningSocketReady(void);
		int		closeSocket(void);
		int		acceptConnection(void);
		void	logIpClient(struct sockaddr_in*	addr);
		
		TStr	putInfoToStr(void) const;
};


#endif