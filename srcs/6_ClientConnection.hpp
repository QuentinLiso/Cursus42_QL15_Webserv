/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:03 by qliso             #+#    #+#             */
/*   Updated: 2025/06/12 19:10:15 by qliso            ###   ########.fr       */
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
		// 0 - Enums
		enum	Status
		{
			CONNECTION_LOST,
			RECV_ERROR,
			REQUEST_TOO_LONG,
			READ_OK
		};

		// 1 - Constructor destructor
		ClientConnection(int fd, const ListeningSocket* relatedListeningSocket);
		virtual ~ClientConnection(void);

	private:
		// 2 - Variables
		int     						_fd;
		const ListeningSocket* const	_relatedListeningSocket;
		HttpRequest						_httpRequest;
		HttpResponse					_httpResponse;
		
		// 3 - Handling request and response after reading
		void	handleCompleteRequest(void);
		void	handleErrorRequest(unsigned short code);
		void	sendResponseBodyFd(int bodyfd);
		void	sendResponseBodyStr(const TStr& bodystr);

	public:
		// 4 - Getter
		int     getFd(void) const;

		// 5 - Reading function called from server loop epoll
		int		readFromFd(void);

		
};



#endif