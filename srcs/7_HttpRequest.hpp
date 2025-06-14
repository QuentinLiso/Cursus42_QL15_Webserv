/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7_HttpRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:15 by qliso             #+#    #+#             */
/*   Updated: 2025/06/14 16:32:20 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"


class	HttpRequest
{
	public:
		// 0 - Enums
		enum	Status
		{
			PARSING_HEADERS,
			PARSING_HEADERS_DONE,
			PARSING_BODY,
			PARSING_BODY_DONE,
			INVALID
		};

		enum	ConnectionType
		{
			CONN_UNSET,
			CONN_KEEP_ALIVE,
			CONN_CLOSE,
			CONN_INVALID
		};

		enum	MediaType
		{
			MEDIA_UNSET,
			MEDIA_TEXT_HTML,
			MEDIA_TEXT_PLAIN,
			MEDIA_APPLICATION_FORM_URLENCODED,
			MEDIA_MULTIPART_FORM_DATA,
			MEDIA_INVALID
		};

		enum	ContentEncodingType
		{
			CE_UNSET,
			CE_IDENTITY,
			CE_INVALID
		};

		enum	TransferEncodingType
		{
			TE_UNSET,
			TE_CHUNKED,
			TE_INVALID	
		};


		// Constructor & destructor
		HttpRequest(void);
		virtual ~HttpRequest(void);

		
		// static const size_t	maxBytes = MAX_REQUEST_LINE + MAX_HEADERS_SIZE + MAX_BODY_SIZE;
		


	private:

		static const size_t	_maxSizeRequestLine;
		static const size_t	_maxSizeUri;
		static const size_t	_maxHeaderCount;
		static const size_t	_maxSizeHeaderLine;
		static const size_t	_maxSizeHeaderName;
		static const size_t	_maxSizeHeaderValue;
		static const size_t	_maxSizeRequestAndHeaders;

		// HttpRequest Logic
		HttpRequest::Status	_status;
		TStr				_buffer;
		size_t				_index;
		size_t				_headersEndIndex;
		unsigned short		_httpStatusCode;
		size_t				_headersSize;
		size_t				_headersCount;
		size_t				_maxBodySize;


		// Request line
		HttpMethods::Type	_method;
		TStr				_uriPath;
		TStr				_uriQuery;
		TStr				_version;

		// Headers - mandatory
		TStr	_hostAddress;		
		ushort	_hostPort;
		uint	_contentLength;		
		
		// Headers - very common
		TStr					_userAgent;
		ConnectionType			_connection;
		MediaType				_contentType;
		TStr					_multipartBoundary;
		std::map<TStr, TStr>	_cookies;
		TStr					_referer;

		// Headers - other
		ContentEncodingType		_contentEncoding;
		TransferEncodingType 	_transferEncoding;

		// Body
		TStr	_body;

		// Parsing Request Line
		bool	parseRequestLine(void);
		bool	parseMethod(const TStr& subRequestLine);
		bool	parseUri(const TStr& subRequestLine);
		static bool	isForbiddenRawByteUriPath(unsigned char c);
		static bool	isForbiddenDecodedByteUriPath(unsigned char c);
		static bool	isForbiddenRawByteUriQuery(unsigned char c);
		static bool	isForbiddenDecodedByteUriQuery(unsigned char c);
		
		bool	checkUriEncoding(TStr& out, const TStr& request, bool (*forbiddenRawBytes)(unsigned char), bool (*forbiddenDecodedBytes)(unsigned char));
		unsigned char	hexLiteralCharsToHexValueChar(unsigned char first, unsigned char second);
		unsigned char	hexLiteralCharToValueChar(unsigned char c);
		bool	isAllowedByte(unsigned char c, const unsigned char* forbiddenBytes);
		bool	isValidUtf8(const TStr& decodedPath);
		bool 	isValidContinuationByte(const unsigned char c);
		bool	parseVersion(const TStr& subRequestLine);

		// Parsing Headers
		bool	parseHeaders(void);
		bool	parseHeaderLine(const TStr& headerLine);
		bool	setHeaderField(const TStr& headerName, const TStr& headerValue);
		bool	setHeaderHost(const TStr& headerValue);
		bool	isValidHostAddress(const TStr& hostAddress);
		bool	setContentLength(const TStr& headerValue);
		bool	setUserAgent(const TStr& headerValue);
		bool	setConnection(const TStr& headerValue);
		bool	setContentType(const TStr& headerValue);
		bool	isValidMediaType(const TStr& mediaType);
		bool	parseMediaParams(const TStr& mediaParams);
		bool	setCookie(const TStr& headerValue);
		bool	isValidCookie(const TStr& cookieKey, const TStr& cookieValue);
		bool	setReferer(const TStr& headerValue);
		bool	setContentEncoding(const TStr& headerValue);
		bool	setTransferEncoding(const TStr& headerValue);

		// Parsing Body
		bool	parseBody(void);		// TO IMPLEMENT
		bool	parseContentLengthBody(void);

		bool	error(unsigned short httpStatusCode, const TStr& step);

	public:

		void	reset(void);

		// Getters
		HttpRequest::Status	getStatus(void) const;
		const TStr&	getBuffer(void) const;
		unsigned short	getStatusCode(void) const;

		HttpMethods::Type getMethod(void) const;
		const TStr& getUriPath(void) const;
		const TStr& getUriQuery(void) const;
		const TStr& getVersion(void) const;
		const TStr& getHostAddress(void) const;
		ushort		getHostPort(void) const;
		uint		getContentLength(void) const;
		const TStr& getUserAgent(void) const;
		ConnectionType getConnection(void) const;
		MediaType getContentType(void) const;
		const TStr& getMultipartBoundary(void) const;
		const std::map<TStr, TStr>& getCookies(void) const;
		const TStr& getReferer(void) const;
		ContentEncodingType	getContentEncoding(void) const;
		TransferEncodingType	getTransferEncoding(void) const;

		// Parsing buffer into an http request
		void				appendToBuffer(char	recvBuffer[], size_t bytes);
		HttpRequest::Status	tryParseHttpRequest(void);
		
		// For testing
		void	setBuffer(const TStr& buffer);
		void	parseRequestAndHeaders(void);

		// Print
		std::ostream&	printRequest(std::ostream& o) const;
};

#endif