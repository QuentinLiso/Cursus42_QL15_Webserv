/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7A_HttpRequestData.hpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:15 by qliso             #+#    #+#             */
/*   Updated: 2025/06/21 18:01:32 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	HTTP_REQUEST_DATA_HPP
# define HTTP_REQUEST_DATA_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"

class	HttpRequestData
{
	public :
		// Constructor & destructor
		HttpRequestData(void);
		virtual ~HttpRequestData(void);

		// 0 - Enums	
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

		static const size_t	_maxSizeRequestLine;
		static const size_t	_maxSizeUri;
		static const size_t	_maxHeaderCount;
		static const size_t	_maxSizeHeaderLine;
		static const size_t	_maxSizeHeaderName;
		static const size_t	_maxSizeHeaderValue;
		static const size_t	_maxSizeRequestAndHeaders;

	private :
		// ===================== FIELDS =====================
		unsigned short		_httpRequestDataStatusCode;
		bool				_validHttpRequestData;

		HttpMethods::Type	_method;
		TStr				_uriPath;
		TStr				_uriQuery;
		TStr				_version;
		TStr				_hostAddress;		
		ushort				_hostPort;
		uint				_contentLength;
		bool				_contentLengthSet;
		TStr					_userAgent;
		ConnectionType			_connection;
		MediaType				_contentType;
		TStr					_multipartBoundary;
		std::map<TStr, TStr>	_cookies;
		TStr					_referer;
		ContentEncodingType		_contentEncoding;
		TransferEncodingType 	_transferEncoding;
		TStr					_body;

		// Request line
		bool			parseMethod(const TStr& subRequestLine);
		bool			parseUri(const TStr& subRequestLine);
		static bool		isForbiddenRawByteUriPath(unsigned char c);
		static bool		isForbiddenDecodedByteUriPath(unsigned char c);
		static bool		isForbiddenRawByteUriQuery(unsigned char c);
		static bool		isForbiddenDecodedByteUriQuery(unsigned char c);		
		bool			checkUriEncoding(TStr& out, const TStr& request, bool (*forbiddenRawBytes)(unsigned char), bool (*forbiddenDecodedBytes)(unsigned char));
		unsigned char	hexLiteralCharsToHexValueChar(unsigned char first, unsigned char second);
		unsigned char	hexLiteralCharToValueChar(unsigned char c);
		bool			isAllowedByte(unsigned char c, const unsigned char* forbiddenBytes);
		bool			isValidUtf8(const TStr& decodedPath);
		bool 			isValidContinuationByte(const unsigned char c);
		bool			parseVersion(const TStr& subRequestLine);

		// Headers
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

		// Error handling
		bool	error(unsigned short httpRequestDataStatusCode, const TStr& step);

	public:
		// Setters
		bool	setRequestLine(const TStr& method, const TStr& uri, const TStr& version);
		bool	setHeaderField(const TStr& headerName, const TStr& headerValue);
		void	resetData(void);
		void	setHttpStatusCode(unsigned short code);

		// Getters
		unsigned short		getHttpRequestDataStatusCode(void) const;
		bool				getValidHttpRequestData(void) const;

		HttpMethods::Type	getMethod(void) const;
		const TStr&			getUriPath(void) const;
		const TStr&			getUriQuery(void) const;
		const TStr&			getVersion(void) const;
		const TStr&			getHostAddress(void) const;
		ushort				getHostPort(void) const;
		uint				getContentLength(void) const;
		bool				isContentLengthSet(void) const;
		const TStr&			getUserAgent(void) const;
		ConnectionType		getConnection(void) const;
		MediaType			getContentType(void) const;
		const TStr&			getMultipartBoundary(void) const;
		const std::map<TStr, TStr>&	getCookies(void) const;
		const TStr&			getReferer(void) const;
		ContentEncodingType		getContentEncoding(void) const;
		TransferEncodingType 	getTransferEncoding(void) const;
		const TStr&				getBody(void) const;

		// Print
		std::ostream&		printRequestData(std::ostream& o) const;
};

#endif