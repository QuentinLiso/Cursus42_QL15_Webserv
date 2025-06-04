/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:03 by qliso             #+#    #+#             */
/*   Updated: 2025/06/04 09:23:54 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_CONNECTION_HPP
# define CLIENT_CONNECTION_HPP

#include "Includes.hpp"
#include "Console.hpp"
#include "0_Utils.hpp"
#include "7_HttpRequest.hpp"


class ClientConnection
{
	public:
		enum	Status
		{
			CONNECTION_LOST,
			RECV_ERROR,
			REQUEST_TOO_LONG,
			READ_OK
		};
		
	private:
		int     	_fd;
		HttpRequest	_httpRequest;
		
	public:
		ClientConnection(int fd);
		virtual ~ClientConnection(void);

		int     getFd(void) const;
		int		readFromFd(void);
};



#endif