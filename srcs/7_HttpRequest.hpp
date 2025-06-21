/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7_HttpRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:15 by qliso             #+#    #+#             */
/*   Updated: 2025/06/22 00:13:45 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"
# include "5_ListeningSocket.hpp"
# include "7A_HttpRequestData.hpp"

class	HttpRequest
{
	public:
		enum	RequestState
		{
			PARSING_HEADERS_NEED_DATA,	// OK
			PARSING_HEADERS_END_FOUND,	// OK
			PARSING_REQUEST_LINE_DONE,	// OK
			PARSING_HEADERS_PROCESSING, // OK
			PARSING_HEADERS_DONE,		// OK
			PARSING_HEADERS_INVALID,	// OK

			PARSING_BODY_FROM_BUFFER,	// OK
			PARSING_BODY_CONTENT_LENGTH_NEED_DATA,	// OK
			PARSING_CHUNK_SIZE,	// OK
			PARSING_CHUNK_DATA,	// OK
			PARSING_CHUNK_LAST_CRLF,	// OK
			PARSING_BODY_DONE,		// OK
			PARSING_BODY_INVALID	// OK
		};

		enum	RequestBodyType
		{
			REQUEST_BODY_NO_BODY,
			REQUEST_BODY_CHUNKED,
			REQUEST_BODY_CONTENT_LENGTH
		};
		
		// Constructor & destructor
		HttpRequest(void);
		virtual ~HttpRequest(void);

	private:
		RequestState		_requestState;
		TStr				_requestBuffer;
		size_t				_index;
		size_t				_headersEndIndex;
		size_t				_headersSize;
		size_t				_headersCount;
		size_t				_maxBodySize;
		size_t				_requestBodySizeCount;
		size_t				_actualChunkedDataSize;
		RequestBodyType		_requestBodyType;
		static uint			_requestBodyParsingFdTmpCount;
		int					_requestBodyParsingFd;
		TStr				_requestBodyParsingFilepath;

		size_t				_currentChunkSize;

		HttpRequestData		_httpRequestData;

		// Headers
		RequestState	findHeadersEnd(void);
		RequestState	parseRequestLine(void);
		RequestState	parseHeaders(void);
		RequestState	parseHeaderLine(const TStr& headerLine);
		RequestState	setResponseBodyType(void);

		// Body
		void			setMaxbodySize(size_t size);		
		RequestState	prepareParsingHttpRequestBody(size_t maxBodySize, bool putStaticRequest, const TStr& resolvedPath);
		RequestState	parseHttpBodyFromRequestBuffer(bool putStaticRequest);
		RequestState	parseChunkedHttpBodyFromRequestBuffer(void);
		bool			setChunkSizeHexValue(const TStr&, size_t& out);
		RequestState	parseContentLengthBody(char recvBuffer[], size_t bytesReceived);
		RequestState	parseChunkedBody(char recvBuffer[], size_t bytesReceived);
		
		// Error handling
		RequestState	error(unsigned short httpStatusCode, const TStr& step, RequestState requestState);

	public:
		// Interface with client connection
		RequestState	parseHttpRequest(char recvBuffer[], size_t bytesReceived);
		RequestState	prepareParseHttpBody(const LocationConfig* locationConfig, bool putStaticRequest, const TStr& resolvedPath);
		RequestState	parseHttpBody(char recvBuffer[], size_t bytesReceived);

		// Getters
		RequestState			getHttpRequestState(void) const;
		const TStr&				getRequestBuffer(void) const;
		const HttpRequestData&	getHttpRequestData(void) const;
		size_t					getRequestBodySize(void) const;
		size_t					getActualChunkedDataSize(void) const;
		const TStr&				getRequestBodyParsingFilepath(void) const;

		// For testing
		void	setBuffer(const TStr& buffer);
};

#endif