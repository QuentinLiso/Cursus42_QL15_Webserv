/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7_HttpRequest.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:15 by qliso             #+#    #+#             */
/*   Updated: 2025/06/03 19:35:22 by qliso            ###   ########.fr       */
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
		enum	Status
		{
			PARSING_REQUEST_LINE,
			PARSING_HEADERS,
			PARSING_BODY,
			COMPLETE,
			INVALID
		};

		static const size_t	maxBytes = 4096;

	private:
		// HttpRequest Logic
		HttpRequest::Status	_status;
		TStr	_buffer;
		size_t	_index;
		
		// Request line
		TStr	_method;
		TStr	_uri;
		TStr	_version;

		// Headers - mandatory
		TStr	_host;
		int		_contentLength;
		
		// Headers - very common
		TStr	_userAgent;
		TStr	_accept;
		TStr	_connection;
		TStr	_contentType;
		TStr	_authorization;
		TStr	_cookie;
		TStr	_referer;
		TStr	_origin;

		// Body
		TStr	_body;

		bool	parseRequestLine(void);	// TO IMPLEMENT
		bool	parseHeaders(void);		// TO IMPLEMENT
		bool	parseBody(void);		// TO IMPLEMENT

	public:
		// Constructor & destructor
		HttpRequest(void);
		virtual ~HttpRequest(void);
		void	reset(void);

		// Getters
		HttpRequest::Status	getStatus(void) const;
		const TStr&	getBuffer(void) const;

		const TStr& getMethod(void) const;
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

		// Print
		std::ostream&	printRequest(std::ostream& o) const;
};

#endif