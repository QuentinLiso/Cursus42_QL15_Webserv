/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7_HttpRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:15 by qliso             #+#    #+#             */
/*   Updated: 2025/06/11 12:10:00 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"

# define MAX_REQUEST_LINE	8192
# define MAX_URI			4096
# define MAX_HEADER_LINE	8192
# define MAX_HEADERS_SIZE	65536
# define MAX_HEADERS_COUNT	100
# define MAX_HEADER_NAME	256
# define MAX_BODY_SIZE		10485760

class	HttpRequest
{
	public:
		enum	Status
		{
			PARSING_REQUEST_LINE,
			PARSING_HEADERS,
			PARSING_BODY,
			COMPLETE,
			INVALID
		};

		static const size_t	maxBytes = MAX_REQUEST_LINE + MAX_HEADERS_SIZE + MAX_BODY_SIZE;
		
	private:
		static const unsigned char	forbiddenRawBytesUriPath[];
		static const unsigned char	forbiddenDecodedBytesUriPath[];
		static const unsigned char	forbiddenRawBytesUriQuery[];
		static const unsigned char	forbiddenDecodedBytesUriQuery[];
		static const bool			allowedHeaderNameChar[128];
		
		// HttpRequest Logic
		HttpRequest::Status	_status;
		TStr				_buffer;
		size_t				_index;
		unsigned short		_httpStatusCode;
		size_t				_headersSize;
		size_t				_headersCount;


		// Request line
		HttpMethods::Type	_method;
		TStr				_uriPath;
		TStr				_uriQuery;
		TStr				_version;

		// Headers - mandatory
		TStr	_hostAddress;		// Valid domain/IP + optional :port No spaces
		ushort	_hostPort;
		int		_contentLength;		//	Valid unsigned integer
		
		// Headers - very common
		TStr	_userAgent;			// Optional but easy -> Check standard allowed chars only
		TStr	_connection;		// Mandatory -> Token list : keep-alive, close
		TStr	_contentType;		// Mandatory if body/cgi -> Specific list type/subtype; charset=UTF-8. Valid MIME
		TStr	_cookie;			// Bonus -> semi colon separated values
		TStr	_referer;			// Optional but easy -> Valid URI but loosely validated

		// Headers - other
		TStr	_contentEncoding;	// Mandatory -> Token list : gzip, deflate, bfr
		TStr	_transferEncoding;	// Mandatory -> chunked, gzip, compress
		TStr	_rangeBytes;				// Optional but easy -> Range: bytes=0-499  or Range: bytes=500-

		// Body
		TStr	_body;

		// Parsing Request Line
		bool	parseRequestLine(void);
		bool	parseMethod(const TStr& subRequestLine);
		bool	parseUri(const TStr& subRequestLine);
		bool	parseVersion(const TStr& subRequestLine);
		bool	checkUriEncoding(TStr& out, const TStr& request, const unsigned char forbiddenRawBytes[], unsigned char const forbiddenDecodedBytes[]);
		unsigned char	hexLiteralCharsToHexValueChar(unsigned char first, unsigned char second);
		unsigned char	hexLiteralCharToValueChar(unsigned char c);
		bool	isAllowedByte(unsigned char c, const unsigned char* forbiddenBytes);
		bool	isValidUtf8(const TStr& decodedPath);
		bool 	isValidContinuationByte(const unsigned char c);

		// Parsing Headers
		bool	parseHeaders(void);
		bool	parseHeaderLine(const TStr& headerLine);
		bool	setHeaderField(const TStr& headerName, const TStr& headerValue);
		bool	setHeaderHost(const TStr& headerValue);
		bool	setContentLength(const TStr& headerValue);
		bool	setUserAgent(const TStr& headerValue);
		bool	setConnection(const TStr& headerValue);
		bool	setContentType(const TStr& headerValue);
		bool	setCookie(const TStr& headerValue);
		bool	setReferer(const TStr& headerValue);
		bool	setContentEncoding(const TStr& headerValue);
		bool	setTransferEncoding(const TStr& headerValue);
		bool	setRangeBytes(const TStr& headerValue);

		// Parsing Body
		bool	parseBody(void);		// TO IMPLEMENT

		bool	isValidHostHeader(const TStr& host);

		bool	error(unsigned short httpStatusCode, const TStr& step);

	public:
		// Constructor & destructor
		HttpRequest(void);
		virtual ~HttpRequest(void);
		void	reset(void);

		// Getters
		HttpRequest::Status	getStatus(void) const;
		const TStr&	getBuffer(void) const;

		HttpMethods::Type getMethod(void) const;
		const TStr& getUri(void) const;
		const TStr& getVersion(void) const;
		const TStr& getHost(void) const;
		int			getContentLength(void) const;
		const TStr& getUserAgent(void) const;
		const TStr& getAccept(void) const;
		const TStr& getConnection(void) const;
		const TStr& getContentType(void) const;
		const TStr& getAuthorization(void) const;
		const TStr& getCookie(void) const;
		const TStr& getReferer(void) const;
		const TStr& getOrigin(void) const;


		// Parsing buffer into an http request
		void	appendToBuffer(char	recvBuffer[], size_t bytes);
		bool	tryParseHttpRequest(void);			// TO IMPLEMENT
		
		// For testing
		bool	setValidRequestForTesting(void);
		void	setBuffer(const TStr& buffer);

		// Print
		std::ostream&	printRequest(std::ostream& o) const;
};

#endif