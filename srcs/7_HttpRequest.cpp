/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7_HttpRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:18 by qliso             #+#    #+#             */
/*   Updated: 2025/06/06 11:20:43 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "7_HttpRequest.hpp"

// // HttpRequest
// bool HttpRequest::parseRequestLine(void)
// {
// 	return (false);
// }

// bool HttpRequest::parseHeaders(void)
// {
// 	return (false);
// }

// bool HttpRequest::parseBody(void)
// {
// 	return (false);
// }


// HttpRequest::HttpRequest(void) : _status(HttpRequest::PARSING_REQUEST_LINE), _index(0), _contentLength(-1) {}

// HttpRequest::~HttpRequest(void) {}

// void	HttpRequest::reset(void)
// {
// 	_status = PARSING_REQUEST_LINE;
// 	_buffer.clear();
// 	_index = 0;

// 	_method = HttpMethods::UNKNOWN;
// 	_uri.clear();
// 	_version.clear();

// 	_host.clear();
// 	_contentLength = -1;
// 	_userAgent.clear();
// 	_accept.clear();
// 	_connection.clear();
// 	_contentType.clear();
// 	_authorization.clear();
// 	_cookie.clear();
// 	_referer.clear();
// 	_origin.clear();
// 	_body.clear();
// }

// HttpRequest::Status HttpRequest::getStatus(void) const { return _status; }
// const TStr& HttpRequest::getBuffer(void) const { return _buffer; }

// HttpMethods::Type HttpRequest::getMethod(void) const { return _method; }
// const TStr& HttpRequest::getUri(void) const { return _uri; }
// const TStr& HttpRequest::getVersion(void) const { return _version; }
// const TStr& HttpRequest::getHost(void) const { return _host; }
// int			HttpRequest::getContentLength(void) const { return _contentLength; }
// const TStr& HttpRequest::getUserAgent(void) const { return _userAgent; }
// const TStr& HttpRequest::getAccept(void) const { return _accept; }
// const TStr& HttpRequest::getConnection(void) const { return _connection; }
// const TStr& HttpRequest::getContentType(void) const { return _contentType; }
// const TStr& HttpRequest::getAuthorization(void) const { return _authorization; }
// const TStr& HttpRequest::getCookie(void) const { return _cookie; }
// const TStr& HttpRequest::getReferer(void) const { return _referer; }
// const TStr& HttpRequest::getOrigin(void) const { return _origin; }

// void	HttpRequest::appendToBuffer(char recvBuffer[], size_t bytes)
// {
// 	_buffer.append(recvBuffer, bytes);
// }

// bool	HttpRequest::tryParseHttpRequest(void)
// {
// 	return (false);
// }

// bool	HttpRequest::setValidRequestForTesting(void)
// {
// 	// HttpRequest Logic
// 	_status = HttpRequest::COMPLETE;
// 	_buffer =
//         "GET /docs HTTP/1.1\r\n"
//         "Host: localhost\r\n"
//         "User-Agent: Mozilla/5.0 (compatible; TestBot/1.0)\r\n"
//         "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
//         "Connection: keep-alive\r\n"
//         "Cookie: sessionid=abc123; csrftoken=xyz456\r\n"
//         "Referer: http://localhost/\r\n"
//         "Origin: http://localhost\r\n"
//         "\r\n";

// 	// Request line
// 	_method = HttpMethods::GET;
// 	_uri = "/docs/notfound.html";
// 	_version = "HTTP/1.1";

// 	// Headers - mandatory
// 	_host = "localhost";
// 	_contentLength = 0;

// 	// Headers - very common
// 	_userAgent = "Mozilla/5.0 (compatible; TestBot/1.0)";
//     _accept = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
//     _connection = "keep-alive";
//     _contentType = "";  // no body
//     _authorization = "";  // no auth in test
//     _cookie = "sessionid=abc123; csrftoken=xyz456";
//     _referer = "http://localhost/";
//     _origin = "http://localhost";

// 	// Body
// 	_body.clear();
	
// 	return (true);
// }

// std::ostream&	HttpRequest::printRequest(std::ostream& o) const
// {
// 	o	<< "Method : " << _method << '\n'
// 		<< "URI : " << _uri << '\n'
// 		<< "Version : " << _version << '\n'
// 		<< "Host : " << _host << '\n'
// 		<< "Content Length : " << _contentLength << '\n'
// 		<< "User Agent : " << _userAgent << '\n'
// 		<< "Accept : " << _accept << '\n'
// 		<< "Connection : " << _connection << '\n'
// 		<< "Content Type : " << _contentType << '\n'
// 		<< "Authorization : " << _authorization << '\n'
// 		<< "Cookie : " << _cookie << '\n'
// 		<< "Refered : " << _referer << '\n'
// 		<< "Origin : " << _origin << '\n'
// 		<< "Body : " << _body << std::endl;
// 	return (o);
// }


// --- Parse request line ---
bool HttpRequest::parseRequestLine(void)
{
    size_t lineEnd = _buffer.find("\r\n", _index);
    if (lineEnd == TStr::npos)
        return false; // Need more data

    if (lineEnd - _index > MAX_REQUEST_SIZE)
    {
        _status = INVALID;
        _status_code = 413; // Payload Too Large
        return false;
    }

    TStr requestLine = _buffer.substr(_index, lineEnd - _index);
    size_t firstSpace = requestLine.find(' ');
    if (firstSpace == TStr::npos)
    {
        _status = INVALID;
        _status_code = 400; // Bad Request
        return false;
    }

    TStr methodStr = requestLine.substr(0, firstSpace);
    if (methodStr == "GET")
        _method = HttpMethods::GET;
    else if (methodStr == "POST")
        _method = HttpMethods::POST;
    else if (methodStr == "PUT")
        _method = HttpMethods::PUT;
    else if (methodStr == "DELETE")
        _method = HttpMethods::DELETE;
    // else if (methodStr == "HEAD")
    //     _method = HttpMethods::HEAD;
    // else if (methodStr == "OPTIONS")
    //     _method = HttpMethods::OPTIONS;
    else
    {
        _method = HttpMethods::UNKNOWN;
        _status = INVALID;
        _status_code = 405; // Method Not Allowed
        return false;
    }

    size_t secondSpace = requestLine.find(' ', firstSpace + 1);
    if (secondSpace == TStr::npos)
    {
        _status = INVALID;
        _status_code = 400; // Bad Request
        return false;
    }

    _uri = requestLine.substr(firstSpace + 1, secondSpace - firstSpace - 1);
    _version = requestLine.substr(secondSpace + 1);

    if (_version != "HTTP/1.1" && _version != "HTTP/1.0")
    {
        _status = INVALID;
        _status_code = 505; // HTTP Version Not Supported
        return false;
    }

    _index = lineEnd + 2; // skip \r\n
    _status = PARSING_HEADERS;

    return true;
}

// --- Parse headers ---
bool HttpRequest::parseHeaders(void)
{
    while (true)
    {
        size_t lineEnd = _buffer.find("\r\n", _index);
        if (lineEnd == TStr::npos)
            return false; // Need more data

        if (lineEnd == _index) // Empty line indicates end of headers
        {
            _index += 2; // skip \r\n

            if (_host.empty())
            {
                _status = INVALID;
                _status_code = 400; // Bad Request - Host header mandatory
                return false;
            }

            if (_contentLength > 0)
                _status = PARSING_BODY;
            else
                _status = COMPLETE;

            return true;
        }

        TStr headerLine = _buffer.substr(_index, lineEnd - _index);
        size_t colonPos = headerLine.find(':');
        if (colonPos == TStr::npos)
        {
            _status = INVALID;
            _status_code = 400; // Bad Request - malformed header
            return false;
        }

        TStr headerName = headerLine.substr(0, colonPos);
        TStr headerValue = headerLine.substr(colonPos + 1);

        // Trim whitespace
        size_t start = headerValue.find_first_not_of(" \t");
        size_t end = headerValue.find_last_not_of(" \t");
        if (start != TStr::npos && end != TStr::npos)
            headerValue = headerValue.substr(start, end - start + 1);
        else if (start != TStr::npos)
            headerValue = headerValue.substr(start);
        else
            headerValue.clear();

        // Convert header name to lowercase
        for (size_t i = 0; i < headerName.length(); ++i)
            headerName[i] = std::tolower(headerName[i]);

        // Save known headers
        if (headerName == "host")
            _host = headerValue;
        else if (headerName == "content-length")
        {
            _contentLength = std::atoi(headerValue.c_str());
            if (_contentLength < 0)
                _contentLength = 0;
        }
        else if (headerName == "user-agent")
            _userAgent = headerValue;
        else if (headerName == "accept")
            _accept = headerValue;
        else if (headerName == "connection")
            _connection = headerValue;
        else if (headerName == "content-type")
            _contentType = headerValue;
        else if (headerName == "authorization")
            _authorization = headerValue;
        else if (headerName == "cookie")
            _cookie = headerValue;
        else if (headerName == "referer")
            _referer = headerValue;
        else if (headerName == "origin")
            _origin = headerValue;

        _index = lineEnd + 2; // skip \r\n
    }

    return true;
}

// --- Parse body ---
bool HttpRequest::parseBody(void)
{
    size_t remainingBytes = _buffer.length() - _index;

    if (static_cast<int>(remainingBytes) < _contentLength)
        return false; // Need more data

    _body = _buffer.substr(_index, _contentLength);
    _index += _contentLength;
    _status = COMPLETE;

    return true;
}

// --- Constructor / Destructor ---
HttpRequest::HttpRequest(void)
    :  _status_code(0),
      _status(HttpRequest::PARSING_REQUEST_LINE),
      _index(0),
      _method(HttpMethods::UNKNOWN),
      _contentLength(-1)
{}

HttpRequest::~HttpRequest(void) {}

void HttpRequest::reset(void)
{
    _status = PARSING_REQUEST_LINE;
    _buffer.clear();
    _index = 0;
    _status_code = 0;

    _method = HttpMethods::UNKNOWN;
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

// --- Getters ---
HttpRequest::Status HttpRequest::getStatus(void) const { return _status; }
const TStr& HttpRequest::getBuffer(void) const { return _buffer; }

HttpMethods::Type HttpRequest::getMethod(void) const { return _method; }
const TStr& HttpRequest::getUri(void) const { return _uri; }
const TStr& HttpRequest::getVersion(void) const { return _version; }
const TStr& HttpRequest::getHost(void) const { return _host; }
int HttpRequest::getContentLength(void) const { return _contentLength; }
const TStr& HttpRequest::getUserAgent(void) const { return _userAgent; }
const TStr& HttpRequest::getAccept(void) const { return _accept; }
const TStr& HttpRequest::getConnection(void) const { return _connection; }
const TStr& HttpRequest::getContentType(void) const { return _contentType; }
const TStr& HttpRequest::getAuthorization(void) const { return _authorization; }
const TStr& HttpRequest::getCookie(void) const { return _cookie; }
const TStr& HttpRequest::getReferer(void) const { return _referer; }
const TStr& HttpRequest::getOrigin(void) const { return _origin; }
int HttpRequest::getStatusCode() const { return _status_code; }

// --- Append data to buffer ---
void HttpRequest::appendToBuffer(char recvBuffer[], size_t bytes)
{
    if (_buffer.length() + bytes > MAX_REQUEST_SIZE)
    {
        _status = INVALID;
        _status_code = 413; // Payload Too Large
        return;
    }
    _buffer.append(recvBuffer, bytes);
}

// --- Try to parse the HTTP request ---
bool HttpRequest::tryParseHttpRequest(void)
{
    while (_status != COMPLETE && _status != INVALID)
    {
        bool success = false;

        switch (_status)
        {
            case PARSING_REQUEST_LINE:
                success = parseRequestLine();
                break;
            case PARSING_HEADERS:
                success = parseHeaders();
                break;
            case PARSING_BODY:
                success = parseBody();
                break;
            default:
                return false;
        }

        if (!success)
            return false;
    }

    return (_status == COMPLETE);
}

// --- For testing ---
bool HttpRequest::setValidRequestForTesting(void)
{
    _status = HttpRequest::COMPLETE;
    _buffer =
        "GET /docs HTTP/1.1\r\n"
        "Host: localhost\r\n"
        "User-Agent: Mozilla/5.0 (compatible; TestBot/1.0)\r\n"
        "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8\r\n"
        "Connection: keep-alive\r\n"
        "Cookie: sessionid=abc123; csrftoken=xyz456\r\n"
        "Referer: http://localhost/\r\n"
        "Origin: http://localhost\r\n"
        "\r\n";

    _method = HttpMethods::GET;
    _uri = "/docs";
    _version = "HTTP/1.1";
    _host = "localhost";
    _contentLength = 0;
    _userAgent = "Mozilla/5.0 (compatible; TestBot/1.0)";
    _accept = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
    _connection = "keep-alive";
    _cookie = "sessionid=abc123; csrftoken=xyz456";
    _referer = "http://localhost/";
    _origin = "http://localhost";
    _status_code = 0;

    return true;
}

// --- Print the request for debug ---
std::ostream& HttpRequest::printRequest(std::ostream& o) const
{
    o << "HTTP Request:" << std::endl;
    // o << "  Method: " << HttpMethods::toString(_method) << std::endl;
    o << "  URI: " << _uri << std::endl;
    o << "  Version: " << _version << std::endl;
    o << "  Host: " << _host << std::endl;
    o << "  Content-Length: " << _contentLength << std::endl;
    o << "  User-Agent: " << _userAgent << std::endl;
    o << "  Accept: " << _accept << std::endl;
    o << "  Connection: " << _connection << std::endl;
    o << "  Content-Type: " << _contentType << std::endl;
    o << "  Authorization: " << _authorization << std::endl;
    o << "  Cookie: " << _cookie << std::endl;
    o << "  Referer: " << _referer << std::endl;
    o << "  Origin: " << _origin << std::endl;
    o << "  Body: " << _body << std::endl;
    o << "  Status Code: " << _status_code << std::endl;
    return o;
}