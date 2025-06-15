/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   8_HttpResponse.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 09:22:48 by qliso             #+#    #+#             */
/*   Updated: 2025/06/15 17:08:58 by qliso            ###   ########.fr       */
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
		enum	Status
		{
			PROCESSING,
			READY_TO_SEND
		};

		enum	BodyType
		{
			NO_BODY,
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
		HttpResponse::Status		_status;
		const HttpRequest*			_httpRequest;
		const LocationConfig*		_locationConfig;
		TStr						_resolvedPath;
		int							_requestResolvedPathStatResult;
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
		TStr	buildResolvedPath(void);
		bool	isResolvedPathAllowed(const TStr& resolvedPath);

		//	GET + HEAD
		HttpResponse::Status	prepareResponseToGetFromHeaders(const TStr& resolvedPath);
		HttpResponse::Status	handleGetDirectory(const TStr& resolvedPath);
		unsigned short			handleGetDirectoryIndex(const TStr& resolvedPath);
		HttpResponse::Status	handleGetDirectoryAutoindex(const TStr& resolvedPath);
		HttpResponse::Status	handleGetFile(const TStr& resolvedPath);
		unsigned short			handleGetCgiFile(const TStr& resolvedPath);
		unsigned short			handleGetStaticFile(const TStr& resolvedPath);

		// DELETE
		HttpResponse::Status	prepareResponseToDeleteFromHeaders(const TStr& resolvedPath);
		TStr					getParentDirectory(const TStr& resolvedPath);
		unsigned short			isDirectoryEmpty(const TStr& resolvedPath);

		// POST + PUT
		HttpResponse::Status	prepareResponseToPutFromHeaders(const TStr& resolvedPath);
		HttpResponse::Status	prepareResponseToPostFromHeaders(const TStr& resolvedPath);
		HttpResponse::Status	prepareResponseToPostFromBody(const TStr& resolvedPath);

		// CGI
		unsigned short	handleCgi(const TStr& resolvedPath);
		void			buildExecveArgs(const TStr& resolvedPath, TStrVect& tmpEnvp, std::vector<char*>& argv, std::vector<char*>& envp);
		unsigned short	prepareResponseFromCgi(const TStr& resolvedPath, const TStr& cgiOutput);
		
		
		// 9 - Handling error requests
		TStr	createDefaultStatusPage(ushort statusCode);
		HttpResponse::Status	handleErrorRequest(ushort statusCode);
		bool	canServeCustomErrorPage(ushort statusCode);

		
	public:
		// 10 - Getters
		HttpResponse::Status	getStatus(void) const;
		BodyType	getBodyType(void) const;
		int			getBodyFd(void) const;
		const TStr&	getBodyStr(void) const;

		// 11 - Set HttpResponse fields from request
		HttpResponse::Status	prepareResponseFromRequestHeadersComplete(const HttpRequest* httpRequest, const LocationConfig* locationConfig);
		HttpResponse::Status	prepareResponseFromRequestBodyComplete(void);
		HttpResponse::Status	prepareResponseInvalidHeadersRequest(unsigned short code);
		HttpResponse::Status	prepareResponseInvalidBodyRequest(unsigned short code);
    	
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

