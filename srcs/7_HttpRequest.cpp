/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7_HttpRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:18 by qliso             #+#    #+#             */
/*   Updated: 2025/06/03 20:01:36 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "7_HttpRequest.hpp"

// HttpRequest
bool HttpRequest::parseRequestLine(void)
{
	return (false);
}

bool HttpRequest::parseHeaders(void)
{
	return (false);
}

bool HttpRequest::parseBody(void)
{
	return (false);
}


HttpRequest::HttpRequest(void) : _status(HttpRequest::PARSING_REQUEST_LINE), _index(0), _contentLength(-1) {}

HttpRequest::~HttpRequest(void) {}

void	HttpRequest::reset(void)
{
	_status = PARSING_REQUEST_LINE;
	_buffer.clear();
	_index = 0;

	_method.clear();
	_uri.clear();
	_version.clear();

	_host.clear();
	_contentLength = -1;
	_userAgent.clear();
	_accept.clear();
	_connection.clear();
	_contentType.clear();
	_authorization.clear();
	_cookie.clear();
	_referer.clear();
	_origin.clear();
	_body.clear();
}

HttpRequest::Status HttpRequest::getStatus(void) const { return _status; }
const TStr& HttpRequest::getBuffer(void) const { return _buffer; }

const TStr& HttpRequest::getMethod(void) const { return _method; }
const TStr& HttpRequest::getUri(void) const { return _uri; }
const TStr& HttpRequest::getVersion(void) const { return _version; }
const TStr& HttpRequest::getHost(void) const { return _host; }
int			HttpRequest::getContentLength(void) const { return _contentLength; }
const TStr& HttpRequest::getUserAgent(void) const { return _userAgent; }
const TStr& HttpRequest::getAccept(void) const { return _accept; }
const TStr& HttpRequest::getConnection(void) const { return _connection; }
const TStr& HttpRequest::getContentType(void) const { return _contentType; }
const TStr& HttpRequest::getAuthorization(void) const { return _authorization; }
const TStr& HttpRequest::getCookie(void) const { return _cookie; }
const TStr& HttpRequest::getReferer(void) const { return _referer; }
const TStr& HttpRequest::getOrigin(void) const { return _origin; }

void	HttpRequest::appendToBuffer(char recvBuffer[], size_t bytes)
{
	_buffer.append(recvBuffer, bytes);
}

bool	HttpRequest::tryParseHttpRequest(void)
{
	return (false);
}

bool	HttpRequest::setValidRequestForTesting(void)
{
	// HttpRequest Logic
	_status = HttpRequest::COMPLETE;
	_buffer =
        "GET / HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: Mozilla/5.0 (compatible; TestBot/1.0)\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Connection: keep-alive\r\n"
        "Cookie: sessionid=abc123; csrftoken=xyz456\r\n"
        "Referer: http://localhost/\r\n"
        "Origin: http://localhost\r\n"
        "\r\n";

	// Request line
	_method = "GET";
	_uri = "/";
	_version = "HTTP/1.1";

	// Headers - mandatory
	_host = "localhost";
	_contentLength = 0;

	// Headers - very common
	_userAgent = "Mozilla/5.0 (compatible; TestBot/1.0)";
    _accept = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
    _connection = "keep-alive";
    _contentType = "";  // no body
    _authorization = "";  // no auth in test
    _cookie = "sessionid=abc123; csrftoken=xyz456";
    _referer = "http://localhost/";
    _origin = "http://localhost";

	// Body
	_body.clear();
	
	return (true);
}

std::ostream&	HttpRequest::printRequest(std::ostream& o) const
{
	o	<< "Method : " << _method << '\n'
		<< "URI : " << _uri << '\n'
		<< "Version : " << _version << '\n'
		<< "Host : " << _host << '\n'
		<< "Content Length : " << _contentLength << '\n'
		<< "User Agent : " << _userAgent << '\n'
		<< "Accept : " << _accept << '\n'
		<< "Connection : " << _connection << '\n'
		<< "Content Type : " << _contentType << '\n'
		<< "Authorization : " << _authorization << '\n'
		<< "Cookie : " << _cookie << '\n'
		<< "Refered : " << _referer << '\n'
		<< "Origin : " << _origin << '\n'
		<< "Body : " << _body << std::endl;
	return (o);
}

