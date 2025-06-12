/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   8_HttpResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 09:22:48 by qliso             #+#    #+#             */
/*   Updated: 2025/06/12 19:10:55 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	HTTP_RESPONSE_HPP
# define HTTP_RESPONSE_HPP

#include "Includes.hpp"
#include "Console.hpp"
#include "0_Utils.hpp"
#include "3_Build.hpp"
#include "7_HttpRequest.hpp"

class	HttpResponse
{
	public :
		// 0 - Enums
		enum	BodyType
		{
			FILEDESCRIPTOR,
			STRING,
			UNKNOWN
		};
	
		// 1 - Contstructor & destructor
		HttpResponse(void);
		virtual ~HttpResponse(void);

	
	private:
		// 2 - Static variables
		static const char* const *	httpStatusCodes;
		static const char**			initHttpStatusCodes(void);
		static const char*			getStatusCodeReason(unsigned short httpStatusCode);

		static std::map<TStr, TStr>	mimeMap;
		static std::map<TStr, TStr>	initMimeMap(void);
		static const TStr&			getMimeType(const TStr& filepath);	

		// 3 - Variables
		const HttpRequest*			_httpRequest;
		const LocationConfig*		_locationConfig;
		struct stat					_requestResolvedPathStatus;
		unsigned short				_statusCode;	
		std::map<TStr, TStr> 		_headers;
		BodyType					_bodyType;
    	TStr 						_body;
		int							_bodyfd;


		// 4 - Set headers that will be present in all requests
		void	setDefaultHeaders(void);

		// 5 - Handling bad requests
		bool	isRequestHttpMethodAllowed(void);
		bool	isResolvedPathAllowed(const TStr& resolvedPath);

		// 6 - Handling well formed requests
		bool	handleResolvedPath(const TStr& resolvedPath);

		// 7 - Handling a request for a directory
		bool	handleRequestedDirectory(const TStr& resolvedPath);
		bool	handleRequestedDirectoryIndexFile(const TStr& filepath);
		bool	handleRequestedDirectoryAutoindex(const TStr& folderpath);

		// 8 - Handling a request for a static file
		bool	handleRequestedFile(const TStr& resolvedPath);
		bool	handleRequestedStaticFile(const TStr& resolvedPath);

		// 9 - Handling error requests
		TStr	createDefaultStatusPage(ushort statusCode);
		void	handleErrorRequest(ushort statusCode);
		bool	canServeCustomErrorPage(ushort statusCode);

		
	public:
		// 10 - Getters
		BodyType	getBodyType(void) const;
		int			getBodyFd(void) const;
		const TStr&	getBodyStr(void) const;

		// 11 - Set HttpResponse fields from request
		void	prepareResponse(const HttpRequest* httpRequest, const LocationConfig* locationConfig);
		void	prepareErrorResponse(unsigned short code);
    	
		// 12 - Convert HttpResponse fields to a string to send
		TStr	toString() const;

		// 13 - Debug printing
		void	print(void) const;

};



#endif


// Status codes
// #define CONTINUE 100
// #define SWITCHING_PROTOCOLS 101
// #define PROCESSING 102
// #define EARLY_HINTS 103
// #define OK 200
// #define CREATED 201
// #define ACCEPTED 202
// #define NON_AUTHORITATIVE_INFORMATION 203
// #define NO_CONTENT 204
// #define RESET_CONTENT 205
// #define PARTIAL_CONTENT 206
// #define MULTI_STATUS 207
// #define ALREADY_REPORTED 208
// #define IM_USED 226
// #define MULTIPLE_CHOICES 300
// #define MOVED_PERMANENTLY 301
// #define FOUND 302
// #define SEE_OTHER 303
// #define NOT_MODIFIED 304
// #define TEMPORARY_REDIRECT 307
// #define PERMANENT_REDIRECT 308
// #define BAD_REQUEST 400
// #define UNAUTHORIZED 401
// #define PAYMENT_REQUIRED 402
// #define FORBIDDEN 403
// #define NOT_FOUND 404
// #define METHOD_NOT_ALLOWED 405
// #define NOT_ACCEPTABLE 406
// #define PROXY_AUTHENTICATION_REQUIRED 407
// #define REQUEST_TIMEOUT 408
// #define CONFLICT 409
// #define GONE 410
// #define LENGTH_REQUIRED 411
// #define PRECONDITION_FAILED 412
// #define CONTENT_TOO_LARGE 413
// #define URI_TOO_LONG 414
// #define UNSUPPORTED_MEDIA_TYPE 415
// #define RANGE_NOT_SATISFIABLE 416
// #define EXPECTATION_FAILED 417
// #define IM_A_TEAPOT 418
// #define MISDIRECTED_REQUEST 421
// #define UNPROCESSABLE_CONTENT 422
// #define LOCKED 423
// #define FAILED_DEPENDENCY 424
// #define TOO_EARLY 425
// #define UPGRADE_REQUIRED 426
// #define PRECONDITION_REQUIRED 428
// #define TOO_MANY_REQUESTS 429
// #define REQUEST_HEADER_FIELDS_TOO_LARGE 431
// #define UNAVAILABLE_FOR_LEGAL_REASONS 451
// #define INTERNAL_SERVER_ERROR 500
// #define NOT_IMPLEMENTED 501
// #define BAD_GATEWAY 502
// #define SERVICE_UNAVAILABLE 503
// #define GATEWAY_TIMEOUT 504
// #define HTTP_VERSION_NOT_SUPPORTED 505
// #define VARIANT_ALSO_NEGOTIATES 506
// #define INSUFFICIENT_STORAGE 507
// #define LOOP_DETECTED 508
// #define NOT_EXTENDED 510
// #define NETWORK_AUTHENTICATION_REQUIRED 511

