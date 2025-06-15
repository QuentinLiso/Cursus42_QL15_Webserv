/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:03 by qliso             #+#    #+#             */
/*   Updated: 2025/06/14 18:49:42 by qliso            ###   ########.fr       */
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
		enum	ReadStatus
		{
			READ_CONNECTION_LOST,
			READ_RECV_ERROR,
			READ_AGAIN_LATER,
			READ_OK,
		};

		enum	WriteStatus
		{
			WRITE_CONNECTION_LOST,
			WRITE_SEND_ERROR,
			WRITE_AGAIN_LATER,
			WRITE_OK
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
		
		// 3 - Preparing response
		HttpResponse::Status	prepareResponseFromRequestHeadersComplete(void);
		ReadStatus				prepareResponseFromRequestBodyComplete(void);
		ReadStatus				prepareResponseInvalidHeadersRequest(unsigned short code, ReadStatus readStatus);
		ReadStatus				prepareResponseInvalidBodyRequest(unsigned short code, ReadStatus readStatus);
		
		// 4 - Sending response
		void	sendResponseHeader(void);
		void	sendResponseBody(void);
		void	sendResponseBodyFd(int bodyfd);
		void	sendResponseBodyStr(const TStr& bodystr);

	public:
		// 4 - Getter
		int     getFd(void) const;

		// 5 - Read from and send to client
		ReadStatus	readFromFd(void);
		WriteStatus	sendToFd(void);

		
};



#endif