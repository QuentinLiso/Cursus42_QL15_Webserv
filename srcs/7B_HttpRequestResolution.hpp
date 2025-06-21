/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7B_HttpRequestResolution.hpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:15 by qliso             #+#    #+#             */
/*   Updated: 2025/06/19 18:13:59 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	HTTP_REQUEST_RESOLUTION_HPP
# define HTTP_REQUEST_RESOLUTION_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"
# include "3_Build.hpp"
# include "5_ListeningSocket.hpp"
# include "7_HttpRequest.hpp"
# include "7A_HttpRequestData.hpp"
# include "8_HttpResponse.hpp"

class	HttpRequestResolution
{
	public:
		enum	ResolutionState
		{
			RESOLUTION_PROCESSING,
			
			RESOLUTION_VALID_GET_HEAD_STATIC,
			RESOLUTION_VALID_GET_HEAD_CGI,
			RESOLUTION_VALID_GET_HEAD_AUTOINDEX,
			
			RESOLUTION_VALID_DELETE,
			
			RESOLUTION_VALID_POST_CGI,
			
			RESOLUTION_VALID_PUT_STATIC,
			RESOLUTION_VALID_PUT_CGI,

			RESOLUTION_INVALID,
		};

	public :
		HttpRequestResolution(const HttpRequest& httpRequest);
		~HttpRequestResolution(void);

	private:
		// 2 - Static variables
		static const char* const *	httpStatusCodes;
		static const char**			initHttpStatusCodes(void);
		static const char*			getStatusCodeReason(unsigned short httpStatusCode);

		static std::map<TStr, TStr>	mimeMap;
		static std::map<TStr, TStr>	initMimeMap(void);
		static const TStr&			getMimeType(const TStr& filepath);	

		const HttpRequest&			_httpRequest;
		const HttpRequestData&		_httpRequestData;
		
		unsigned short				_httpResolutionStatusCode;
		ResolutionState				_resolutionState;
		const LocationConfig*		_locationConfig;
		TStr						_resolvedPath;
		int							_resolvedPathExists;
		struct stat					_resolvedPathStat;
		TStr						_resolvedPathParentDir;
		
		ResolutionState	isRequestHttpMethodAllowed(void);
		TStr			buildResolvedPath(void);
		ResolutionState	isResolvedPathAllowed(void);
		TStr			getParentDirectory(const TStr& path);
		
		ResolutionState	isGetHeadRequestValid(void);
		ResolutionState	handleGetHeadFile(void);
		ResolutionState	handleGetHeadDirectory(void);

		ResolutionState	isDeleteRequestValid(void);
		ResolutionState	isDirectoryEmpty(void);

		ResolutionState	isPutRequestValid(void);
		ResolutionState	isPostRequestValid(void);

		// Error handling
		ResolutionState	error(ushort httpResolutionStatusCode, const TStr& step, ResolutionState resolutionState);

	public:
		ResolutionState	resolveHttpRequest(const ListeningSocket* const listeningSocket);

		// Getters
		unsigned short				getHttpResolutionStatusCode(void) const;
		ResolutionState				getResolutionState(void) const;
		const LocationConfig*		getLocationConfig(void) const;
		const TStr&					getResolvedPath(void) const;
		const struct stat&			getResolvedPathStat(void) const;
		const TStr&					getParentDirectory(void) const; 

		// Print
		std::ostream&	printRequest(std::ostream& o) const;

};

#endif