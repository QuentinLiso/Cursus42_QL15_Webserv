/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   6_ClientConnection.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/02 13:04:03 by qliso             #+#    #+#             */
/*   Updated: 2025/06/24 06:28:47 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CLIENT_CONNECTION_HPP
# define CLIENT_CONNECTION_HPP

#include "Includes.hpp"
#include "Console.hpp"
#include "0_Utils.hpp"
#include "4_Server.hpp"
#include "5_ListeningSocket.hpp"
#include "7_HttpRequest.hpp"
#include "7B_HttpRequestResolution.hpp"
#include "7C_CgiHandler.hpp"
#include "8_HttpResponse.hpp"

# define DISCARD_TMP false
# define LOG_BYTES_SENT false

class Server;

class ClientConnection
{
	public:
		enum	ClientState
		{
			STATE_READING_HEADERS,
			STATE_REQUEST_RESOLUTION,
			STATE_PREPARE_READING_BODY,
			STATE_READING_BODY,
			STATE_READY_TO_SEND,
			STATE_SENDING_BODY_STR,
			STATE_SENDING_BODY_FD,
			
			
			STATE_SENDING_RESPONSE,
			STATE_CLOSING_CONNECTION,
			STATE_CGI_PREPARE,
			STATE_CGI_READY,
			STATE_CGI_FINISHED,
			STATE_PAUSE,
		};

		enum	SendState
		{
			SENDING_HEADERS,
			SENDING_BODY_STRING,
			SENDING_BODY_FD,
			SENDING_DONE
		};

		ClientConnection(Server& server, int fd, const ListeningSocket* relatedListeningSocket);
		virtual ~ClientConnection(void);

	private:
		// 2 - Variables
		Server&							_server;
		int     						_fd;
		const ListeningSocket* const	_relatedListeningSocket;
		ClientState						_clientConnectionState;
		bool							_needEpollToProgress;

		HttpRequest					_httpRequest;
		HttpRequestResolution		_httpResolution;
		CgiHandler					_cgiHandler;
		HttpResponse				_httpResponse;

		SendState					_sendState;
		TStr						_sendBuffer;
		size_t						_sendOffset;
		int							_sendFd;
		size_t						_actualBytesSent;
		bool						_sendFdClear;

		static int 					_logBytesSentFilecount;
		TStr						_logBytesSentFilename;
		int							_logBytesSentFd;

		// Events functions
		void	handleReadingHeaders(int events, int fd, FdType::Type fdType);
		void	handleReadingHeadersInvalid(void);
		void	handleReadingHeadersDone(void);
		void	handleClientDisconnectedWhileRecv(void);
		void	handleRecvError(void);

		void	handleRequestValidation(void);
		void	handleValidGetHeadStatic(void);
		void	handleValidGetHeadCgi(void);
		void	handleValidGetHeadAutoindex(void);
		void	handleValidDelete(void);
		void	handleValidPostPut(void);
		void	handleInvalidRequest(void);

		void	handlePrepareReadingBody(void);
		void	handlePrepareReadingBodyNeedsData(void);
		void	handleParsingBodyDone(void);
		void	handleReadingBodyInvalid(void);

		void	handleReadingBody(int events, int fd, FdType::Type fdType);

		void	handleCgiPrepare(void);
		void	handleCgiPrepareValid(void);
		void	handleCgiPrepareError(void);

		void	handleCgiReady(int events, int fd, FdType::Type fdType);
		void	handleCgiFinished(void);
		


		void	handleReadyToSend(void);
		void	handleSendingStr(int events, FdType::Type fdType);
		void	handleSendingFd(int events, FdType::Type fdType);

		void	handleClosingConnection(void);


	public:
		// Event handler
		void	handleEvent(int events, int fd, FdType::Type fdType);

		// 4 - Getter
		int     			getFd(void) const;
		ClientState			getClientConnectionState(void) const;
		const CgiHandler& 	getCgiHandler(void) const;
		bool				needEpollEventToProgress(void) const;
		
		// void	setClientConnectionSendingResponse(void);
		// void	setClientConnectionCgiReady(void);

	
};



#endif