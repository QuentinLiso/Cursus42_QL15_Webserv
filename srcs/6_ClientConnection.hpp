/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:03 by qliso             #+#    #+#             */
/*   Updated: 2025/06/06 10:56:25 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_CONNECTION_HPP
# define CLIENT_CONNECTION_HPP

#include "Includes.hpp"
#include "Console.hpp"
#include "0_Utils.hpp"
#include "5_ListeningSocket.hpp"
#include "7_HttpRequest.hpp"
#include "8_HttpResponse.hpp"


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
		int     						_fd;
		const ListeningSocket* const	_relatedListeningSocket;
		HttpRequest						_httpRequest;
		HttpResponse					_httpResponse;
		
		void	handleCompleteRequest(void);
		void	sendResponseBodyFd(int bodyfd);
		void	sendResponseBodyStr(const TStr& bodystr);

	public:
		ClientConnection(int fd, const ListeningSocket* relatedListeningSocket);
		virtual ~ClientConnection(void);

		int     getFd(void) const;
		int		readFromFd(void);

		
};



#endif