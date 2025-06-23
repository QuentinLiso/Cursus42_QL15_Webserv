/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   8_HttpResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 09:22:48 by qliso             #+#    #+#             */
/*   Updated: 2025/06/23 07:58:40 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

#include "Includes.hpp"
#include "Console.hpp"
#include "0_Utils.hpp"
#include "3_Build.hpp"

class	HttpResponse
{
	public:
		enum	ResponseBodyType
		{
			BODY_NONE,
			BODY_STRING,
			BODY_FILE_DESCRIPTOR,
			BODY_CGI
		};

		enum	ErrorOrigin
		{
			ERROR_NONE,
			ERROR_HEADER_PARSING,
			ERROR_REQUEST_RESOLUTION,
			ERROR_BODY_READING,
			ERROR_CGI_EXECUTION,
			ERROR_RESPONSE_BUILDING
		};

	HttpResponse(void);
	~HttpResponse(void);

	private:
		// 2 - Static variables
		static const char* const *	httpStatusCodesReasons;
		static const char**			initHttpStatusCodes(void);
		static const char*			getStatusCodeReason(unsigned short httpStatusCode);

		static std::map<TStr, TStr>	mimeMap;
		static std::map<TStr, TStr>	initMimeMap(void);
		static const TStr&			getMimeType(const TStr& filepath);	

	public:
		// 3 - Variables
		unsigned short				_responseStatusCode;
		TStr						_reasonPhrase;
		std::map<TStr, TStr> 		_headers;
		size_t						_headersLength;
		size_t						_bodyLength;
		ResponseBodyType			_responseBodyType;
		int							_responseBodyStaticFd;
		TStr						_responseBodyStr;
		ErrorOrigin					_errorOrigin;	



		void	setDefaultHeaders(void);
		void	setContentLengthHeader(int contentLength);
		void	setContentTypeHeader(const TStr& filename);
		
	public:
		void	setCustomErrorPage(int responseStatusCode, const LocationConfig* locationConfig, ErrorOrigin errorOrigin);
		void	setDefaultErrorPage(int responseStatusCode, ErrorOrigin errorOrigin);
		void	setGetHeadStaticResponse(int responseStatusCode, const TStr& resolvedPath, const struct stat& resolvedPathStat, const LocationConfig* locationConfig, HttpMethods::Type method);
		void	setGetHeadAutoindexResponse(int responseStatusCode, const TStr& resolvedPath, const LocationConfig* locationConfig, HttpMethods::Type method);
		void	setDeleteResponse(int responseStatusCode);
		void	setPutStaticResponse(int responseStatusCode);
		void	setCgiResponse(int responseStatusCode, const TStr& cgiOutputFilePath, int contentSize, const std::map<TStr, TStr>& cgiHeaders, const LocationConfig* locationConfig);

		TStr				headersToString(void);
		size_t				getHeadersLength(void) const;
		size_t				getBodyLength(void) const;
		ResponseBodyType	getResponseBodyType(void) const;
		const TStr&			getResponseBodyStr(void) const;
		int					getResponseBodyFd(void) const;
		int					getContentLength(void) const;

		void	print(void) const;

};



#endif