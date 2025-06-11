/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7_HttpRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:18 by qliso             #+#    #+#             */
/*   Updated: 2025/06/11 12:21:26 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "7_HttpRequest.hpp"

// Static variables

const unsigned char	HttpRequest::forbiddenRawBytesUriPath[] = {
	0x00,	// NULL BYTE
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x7F, // Control chars
	0x20,0x22,0x23,0x25,0x3C,0x3E,0x3F,0x5B,0x5C,0x5D,0x5E,0x60,0x7B,0x7C,0x7D // SPACE "#%<>?[\]^`{|}
};

const unsigned char	HttpRequest::forbiddenDecodedBytesUriPath[] = { 
	0x00, // NULL BYTE
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F, 0x10,0x11,0x12,0x13, 0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x7F, // Control chars
	0x22,0x23,0x25,0x2F,0x3C,0x3E,0x3F, 0x5B,0x5C,0x5D,0x5E,0x60,0x7B,0x7C,0x7D // "#%/<>?[\]^`{|}
};

const unsigned char	HttpRequest::forbiddenRawBytesUriQuery[] = {
	0x00,	// NULL BYTE
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x7F, // Control chars
	0x20,0x22,0x23,0x25,0x3C,0x3E,0x5B,0x5C,0x5D,0x5E,0x60,0x7B,0x7C,0x7D // SPACE "#%<>[\]^`{|}
};

const unsigned char	HttpRequest::forbiddenDecodedBytesUriQuery[] = {
	0x00,	// NULL BYTE
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,0x7F // Control chars
};

const bool			HttpRequest::allowedHeaderNameChar[128] = {
	false, // \0 - ASCII 0
	false, // SOH - ASCII 1
	false, // STX - ASCII 2
	false, // ETX - ASCII 3
	false, // EOT - ASCII 4
	false, // ENQ - ASCII 5
	false, // ACK - ASCII 6
	false, // \a - ASCII 7
	false, // \b - ASCII 8
	false, // \t - ASCII 9
	false, // \n - ASCII 10
	false, // \v - ASCII 11
	false, // \f - ASCII 12
	false, // \r - ASCII 13
	false, // SO - ASCII 14
	false, // SI - ASCII 15
	false, // DLE - ASCII 16
	false, // DC1 - ASCII 17
	false, // DC2 - ASCII 18
	false, // DC3 - ASCII 19
	false, // DC4 - ASCII 20
	false, // NAK - ASCII 21
	false, // SYN - ASCII 22
	false, // ETB - ASCII 23
	false, // CAN - ASCII 24
	false, // EM - ASCII 25
	false, // SUB - ASCII 26
	false, // ESC - ASCII 27
	false, // FS - ASCII 28
	false, // GS - ASCII 29
	false, // RS - ASCII 30
	false, // US - ASCII 31
	false, // SPACE - ASCII 32
	true,  // ! - ASCII 33
	false, // " - ASCII 34
	true,  // # - ASCII 35
	true,  // $ - ASCII 36
	true,  // % - ASCII 37
	true,  // & - ASCII 38
	true,  // ’ - ASCII 39
	false, // ( - ASCII 40
	false, // ) - ASCII 41
	true,  // * - ASCII 42
	true,  // + - ASCII 43
	true,  // , - ASCII 44
	true,  // - - ASCII 45
	true,  // . - ASCII 46
	false, // / - ASCII 47
	true,  // 0 - ASCII 48
	true,  // 1 - ASCII 49
	true,  // 2 - ASCII 50
	true,  // 3 - ASCII 51
	true,  // 4 - ASCII 52
	true,  // 5 - ASCII 53
	true,  // 6 - ASCII 54
	true,  // 7 - ASCII 55
	true,  // 8 - ASCII 56
	true,  // 9 - ASCII 57
	false, // : - ASCII 58
	false, // ; - ASCII 59
	false, // < - ASCII 60
	false, // = - ASCII 61
	false, // > - ASCII 62
	false, // ? - ASCII 63
	false, // @ - ASCII 64
	true,  // A - ASCII 65
	true,  // B - ASCII 66
	true,  // C - ASCII 67
	true,  // D - ASCII 68
	true,  // E - ASCII 69
	true,  // F - ASCII 70
	true,  // G - ASCII 71
	true,  // H - ASCII 72
	true,  // I - ASCII 73
	true,  // J - ASCII 74
	true,  // K - ASCII 75
	true,  // L - ASCII 76
	true,  // M - ASCII 77
	true,  // N - ASCII 78
	true,  // O - ASCII 79
	true,  // P - ASCII 80
	true,  // Q - ASCII 81
	true,  // R - ASCII 82
	true,  // S - ASCII 83
	true,  // T - ASCII 84
	true,  // U - ASCII 85
	true,  // V - ASCII 86
	true,  // W - ASCII 87
	true,  // X - ASCII 88
	true,  // Y - ASCII 89
	true,  // Z - ASCII 90
	false, // [ - ASCII 91
	false, // \ - ASCII 92
	false, // ] - ASCII 93
	true,  // ^ - ASCII 94
	true,  // _ - ASCII 95
	true,  // ` - ASCII 96
	true,  // a - ASCII 97
	true,  // b - ASCII 98
	true,  // c - ASCII 99
	true,  // d - ASCII 100
	true,  // e - ASCII 101
	true,  // f - ASCII 102
	true,  // g - ASCII 103
	true,  // h - ASCII 104
	true,  // i - ASCII 105
	true,  // j - ASCII 106
	true,  // k - ASCII 107
	true,  // l - ASCII 108
	true,  // m - ASCII 109
	true,  // n - ASCII 110
	true,  // o - ASCII 111
	true,  // p - ASCII 112
	true,  // q - ASCII 113
	true,  // r - ASCII 114
	true,  // s - ASCII 115
	true,  // t - ASCII 116
	true,  // u - ASCII 117
	true,  // v - ASCII 118
	true,  // w - ASCII 119
	true,  // x - ASCII 120
	true,  // y - ASCII 121
	true,  // z - ASCII 122
	false, // { - ASCII 123
	true,  // | - ASCII 124
	false, // } - ASCII 125
	true,  // ~ - ASCII 126
	false, // DEL - ASCII 127
};

// HttpRequest - Parsing Request Line

bool HttpRequest::parseRequestLine(void)
{
	size_t	lineEnd = _buffer.find("\r\n", _index);
	
	if (lineEnd == std::string::npos)
	{
		if (_buffer.size() > MAX_REQUEST_LINE)
			return (error(413, "Parsing request line")); // \r\n not found but max capacity reached
		return (false);		// \r\n not found, need additional bytes to proceed
	}

	TStr	requestLine = _buffer.substr(_index, lineEnd - _index);
	if (requestLine.size() > MAX_REQUEST_LINE)
		return (error(413, "Parsing request line"));

	size_t	firstSpace = requestLine.find(' ', 0);
	if (firstSpace == 0 || firstSpace == std::string::npos)
		return (error(400, "Parsing request line"));

	size_t	secondSpace = requestLine.find(' ', firstSpace + 1);
	if (secondSpace == (firstSpace + 1) || secondSpace == std::string::npos)
		return (error(400, "Parsing request line"));

	if (requestLine.find(' ', secondSpace + 1) != std::string::npos)
		return (error(400, "Parsing request line"));

	if (!parseMethod(requestLine.substr(0, firstSpace)) 
		|| !parseUri(requestLine.substr(firstSpace + 1, secondSpace - firstSpace))
		|| !parseVersion(requestLine.substr(secondSpace + 1)))
		return (false);

	_index = lineEnd + 2;
	return (true);
}

bool	HttpRequest::parseMethod(const TStr& subRequestLine)
{
	static std::map<TStr, HttpMethods::Type>	httpMethods;
	
	if (httpMethods.empty())
	{
		httpMethods["GET"] = HttpMethods::GET;
		httpMethods["POST"] = HttpMethods::POST;
		httpMethods["DELTE"] = HttpMethods::DELETE;
		httpMethods["PUT"] = HttpMethods::PUT;
	}

	std::map<TStr, HttpMethods::Type>::const_iterator	it = httpMethods.find(subRequestLine);
	if (it == httpMethods.end())
		return (error(405, "Parsing method"));
	
	_method = it->second;
	return (true);
}

bool	HttpRequest::parseUri(const TStr& subRequestLine)
{
	if (subRequestLine.size() > MAX_URI)
		return (error(413, "Parsing URI"));

	size_t	questionMark = subRequestLine.find('?');
	
	TStr	uriPath;
	TStr	uriQuery;

	if (questionMark == std::string::npos)
	{
		if (!checkUriEncoding(uriPath, subRequestLine, HttpRequest::forbiddenRawBytesUriPath, HttpRequest::forbiddenDecodedBytesUriPath))
			return (false);
		_uriPath = uriPath;
	}
	else if (questionMark == 0)
	{
		return (error(400, "Parse URI"));
	}
	else
	{
		if (!checkUriEncoding(uriPath, subRequestLine.substr(0, questionMark), HttpRequest::forbiddenRawBytesUriPath, HttpRequest::forbiddenDecodedBytesUriPath))
			return (false);
		if (!checkUriEncoding(uriQuery, subRequestLine.substr(questionMark + 1), HttpRequest::forbiddenRawBytesUriQuery, HttpRequest::forbiddenDecodedBytesUriQuery))
			return (false);
		_uriPath = uriPath;
		_uriQuery = uriQuery;
	}
	return (true);	
}

bool	HttpRequest::checkUriEncoding(TStr& decodedPath, const TStr& request, const unsigned char forbiddenRawBytes[], unsigned char const forbiddenDecodedBytes[])
{
	if (request.size() > 4096)
		return (error(414, "Uri encoding"));	// uriPath too long -> error 414
	
	decodedPath.reserve(request.size());
	
	for (size_t i = 0; i < request.size(); i++)
	{
		char c = request[i];
		if (c  == '%')
		{
			if (i + 2 >= request.size() || !std::isxdigit(request[i + 1]) || !std::isxdigit(request[i + 2]))
				return (error(400, "Uri encoding"));	// 400 Bad request invalid use of %-encoding (no %XX hex value)

			unsigned char	decodedChar = hexLiteralCharsToHexValueChar(request[i+1], request[i+2]);
			if (!isAllowedByte(c, forbiddenDecodedBytes))
				return (error(400, "Uri encoding"));	// 400 Bad request decoded char is not allowed in URI path

			decodedPath += decodedChar;
			i += 2;
		}
		else
		{
			if (!isAllowedByte(c, forbiddenRawBytes))
				return (error(400, "Uri encoding"));	// 400 bad request invalid raw char
			decodedPath += c;
		}
	}
	if (!isValidUtf8(decodedPath))
		return (false);

	return (true);
}

unsigned char	HttpRequest::hexLiteralCharsToHexValueChar(unsigned char first, unsigned char second)
{
	unsigned char	hexFirst = hexLiteralCharToValueChar(first);		// value between 0 and 15 i.e 0000XXXX
	unsigned char	hexSecond = hexLiteralCharToValueChar(second);		// idem
	return ((hexFirst << 4 ) | hexSecond);								// 0000XXXX << 4 | 0000XXXX
}

unsigned char	HttpRequest::hexLiteralCharToValueChar(unsigned char c)
{
	if (c >= '0' && c <= '9') return (c - '0');
	if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
	return (0xFF);
}

bool	HttpRequest::isAllowedByte(unsigned char c, const unsigned char* forbiddenBytes)
{
	for (size_t i = 0; i < sizeof(forbiddenBytes); i++)
		if (c == forbiddenBytes[i])
			return (false);
	return (true);
}

bool	HttpRequest::isValidUtf8(const TStr& decodedPath)
{
	const unsigned char*	bytes = (const unsigned char*)decodedPath.c_str();
	size_t	len = decodedPath.size();
	size_t	i = 0;

	while (i < len)
	{
		unsigned char 	byte1 = bytes[i];
		
		if (byte1 <= 0x7f)
		{
			// Byte: 0xxxxxxx     (7 bits of data)
			// Code point : [00000000 00000000 00000000 0xxxxxxx]
			i++;
			continue ;
		}

		else if (byte1 >= 0xC0 && byte1 <= 0xDF)
		{
			// Bytes: 110xxxxx 10yyyyyy     (5 + 6 = 11 bits of data)
			// Code point : [00000000 00000000 00000xxx xxyyyyyy]
			if (i + 1 >= len)
				return (false);
			
			unsigned char	byte2 = bytes[i + 1];
			if (!isValidContinuationByte(byte2))
				return (false);
	
			unsigned int	codepoint = ((byte1 & 0x1F) << 6 | (byte2 & 0x3F));
			if (codepoint < 0x80)
				return (false);
			i += 2;
			continue ;
		}

		else if (byte1 >= 0xE0 && byte1 <= 0xEF)
		{
			// Bytes: 1110xxxx 10yyyyyy 10zzzzzz     (4 + 6 + 6 = 16 bits)
			// Code point : [00000000 00000000 xxxxyyyy yyzzzzzz]
			if (i + 2 >= len)
				return (false);

			unsigned char	byte2 = bytes[i + 1];
			unsigned char	byte3 = bytes[i + 2];

			if (!isValidContinuationByte(byte2) || !isValidContinuationByte(byte3))
				return (false);

			unsigned int	codepoint = ((byte1 & 0x0F) << 12 | (byte2 & 0x3F) << 6 | (byte3 & 0x3F));
			if (codepoint < 0x800 || (codepoint >= 0xD800 && codepoint <= 0xDFFF))
				return (false);
			
			i += 3;
			continue ;
		}
		
		else if (byte1 >= 0xF0 && byte1 <= 0xF4)
		{
			// Bytes: 11110www 10xxxxxx 10yyyyyy 10zzzzzz     (3 + 6 + 6 + 6 = 21 bits)
			// Code point : [00000000 000wwwxx xxxxyyyy yyzzzzzz]
			if (i + 3 >= len)
				return (false);

			unsigned char	byte2 = bytes[i + 1];
			unsigned char	byte3 = bytes[i + 2];
			unsigned char	byte4 = bytes[i + 3];

			if (!isValidContinuationByte(byte2) || !isValidContinuationByte(byte3) || !isValidContinuationByte(byte4))
				return (false);

			unsigned int	codepoint = ((byte1 & 0x07) << 18 | (byte2 & 0x3F) << 12 | (byte3 & 0x3F) << 6 | (byte4 & 0x3F));
			if (codepoint < 0x10000 || codepoint > 0x10FFFF)
				return (false);
			
			i += 4;
			continue ;
		}
		else
			return (false);
	}
	return (true);
}

bool 	HttpRequest::isValidContinuationByte(unsigned char c)
{
	return (c >= 0x80 && c <= 0xBF);
}

bool	HttpRequest::parseVersion(const TStr& subRequestLine)
{
	if (subRequestLine != "HTTP/1.1" && subRequestLine != "HTTP/1.0")
		return (error(505, "Parsing version"));
	return (true);
}


bool HttpRequest::parseHeaders(void)
{
	while (true)
	{
		size_t	lineEnd = _buffer.find("\r\n", _index);
				
		if (lineEnd == std::string::npos)
		{
			if (_buffer.size() - _index > MAX_HEADER_LINE)	// CRLF not found and line already too long
				return (error(431, "Parsing headers"));	
			return (false);									// CRLF not found yet
		}
		
		size_t	lineSize = lineEnd - _index;
		if (lineSize == 0)		// CRLF found on an empty line -> end of headers
		{
			_index += 2;
			return (true);
		}

		_headersSize += lineSize;
		_headersCount++;
		if (_headersSize > MAX_HEADERS_SIZE || _headersCount > MAX_HEADERS_COUNT)
			return (error(431, "Parsing headers"));

		if (!parseHeaderLine(_buffer.substr(_index, lineSize)))
			return (false);
		
		_index += 2;
	}
}

bool	HttpRequest::parseHeaderLine(const TStr& headerLine)
{
	if (headerLine.size() > MAX_HEADER_LINE)
		return (error(431, "Parsing header line"));

	size_t	colon = headerLine.find(':');

	if (colon == 0 || colon == std::string::npos)
		return (error(400, "Parsing header line"));
	if (colon >= MAX_HEADER_NAME)
		return (error(431, "Parsing header line, header name too long"));

	TStr	headerName;
	headerName.reserve(colon);
	for (size_t i = 0; i < colon; i++)
		headerName += std::tolower(headerLine[i]);
	TStr	headerValue = headerLine.substr(colon + 1);

	// Trim \t and whitespace
	size_t	start = headerValue.find_first_not_of(" \t");
	size_t	end = headerValue.find_last_not_of(" \t");
	if (start != TStr::npos)
	{
		if (end != TStr::npos)
			headerValue = headerValue.substr(start, end - start + 1);
		else
			headerValue = headerValue.substr(start);
	}
	else
		headerValue.clear();

	return (setHeaderField(headerName, headerValue));
}

bool	HttpRequest::setHeaderField(const TStr& headerName, const TStr& headerValue)
{
	static std::map<TStr, bool (HttpRequest::*)(const TStr&)>	setHeaderFieldMap;
	if (setHeaderFieldMap.empty())
	{
		setHeaderFieldMap["host"] = &HttpRequest::setHeaderHost;
		setHeaderFieldMap["content-length"] = &HttpRequest::setContentLength;
		setHeaderFieldMap["user-agent"] = &HttpRequest::setUserAgent;
		setHeaderFieldMap["connection"] = &HttpRequest::setConnection;
		setHeaderFieldMap["content-type"] = &HttpRequest::setContentType;
		setHeaderFieldMap["cookie"] = &HttpRequest::setCookie;
		setHeaderFieldMap["referer"] = &HttpRequest::setReferer;
		setHeaderFieldMap["content-encoding"] = &HttpRequest::setContentEncoding;
		setHeaderFieldMap["transfer-encoding"] = &HttpRequest::setTransferEncoding;
		setHeaderFieldMap["range"] = &HttpRequest::setRangeBytes;
	}

	std::map<TStr, bool (HttpRequest::*)(const TStr&)>::const_iterator	it = setHeaderFieldMap.find(headerName);
	if (it == setHeaderFieldMap.end())
	{
		for (size_t i = 0; i < headerName.size(); i++)
		{
			unsigned char	c = static_cast<unsigned char>(headerName[i]);
			if (c > 127 || HttpRequest::allowedHeaderNameChar[c] == false)
				return (error(400, "Set header field, invalid char found in header name"));
		}
		for (size_t i = 0; i < headerValue.size(); i++)
			if (headerValue[i] > 126 || headerValue[i] < 32)
				return (error(400, "Set header field, invalid char found in header value"));
		return (true);
	}
	return ((this->*(it->second))(headerValue));
}

bool	HttpRequest::setHeaderHost(const TStr& headerValue)
{
	
}

bool	HttpRequest::setContentLength(const TStr& headerValue)
{
	
}

bool	HttpRequest::setUserAgent(const TStr& headerValue)
{
	
}

bool	HttpRequest::setConnection(const TStr& headerValue)
{
	
}

bool	HttpRequest::setContentType(const TStr& headerValue)
{
	
}

bool	HttpRequest::setCookie(const TStr& headerValue)
{
	
}

bool	HttpRequest::setReferer(const TStr& headerValue)
{
	
}

bool	HttpRequest::setContentEncoding(const TStr& headerValue)
{
	
}

bool	HttpRequest::setTransferEncoding(const TStr& headerValue)
{
	
}

bool	HttpRequest::setRangeBytes(const TStr& headerValue)
{
	
}







bool HttpRequest::parseBody(void)
{
	return (false);
}

// Checks
bool	HttpRequest::isValidHostHeader(const TStr& hostValue)
{
	if (hostValue.empty())
		return (false);

	size_t	sep = hostValue.find_first_of(':');
	static const TStr	validAddressChars = "-.0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
	
	if (sep == hostValue.size() - 1)
		return (false);
	else if (sep == std::string::npos)
	{
		if (hostValue[0] == '-' || hostValue[hostValue.size() - 1] == '-')
			return (false);

		for (size_t i = 0; i < hostValue.size(); i++)
		{
			if (validAddressChars.find(hostValue[i]) == std::string::npos)
				return (false);
		}
		_hostAddress = hostValue;
		return (true);
	}
	else
	{
		TStr			hostAddress = hostValue.substr(0, sep);
		TStr			hostPort = hostValue.substr(sep + 1);
		
		if (hostAddress[0] == '-' || hostAddress[hostAddress.size() - 1] == '-')
			return (false);

		for (size_t i = 0; i < hostAddress.size(); i++)
		{
			if (validAddressChars.find(hostAddress[i]) == std::string::npos)
				return (false);
		}
		_hostAddress = hostValue;

		unsigned short	portValue;
		bool			isValidPort = strToVal<u_int16_t>(hostPort, portValue);

		if (!isValidPort || portValue == 0)
			return (false);
		_hostPort = portValue;
		return (true);
	}
}


// Error
bool	HttpRequest::error(unsigned short httpStatusCode, const TStr& step)
{
	std::ostringstream	oss;
	oss << "Invalid request : " << step << " - Error code : " << httpStatusCode;
	Console::log(Console::DEBUG, oss.str());
	
	_status = HttpRequest::INVALID;
	_httpStatusCode = httpStatusCode;
	return (false);
}

// Contructor & destructor

HttpRequest::HttpRequest(void) : _status(HttpRequest::PARSING_REQUEST_LINE), _index(0), _contentLength(-1) {}

HttpRequest::~HttpRequest(void) {}

void	HttpRequest::reset(void)
{
	_status = PARSING_REQUEST_LINE;
	_buffer.clear();
	_index = 0;

	_method = HttpMethods::UNKNOWN;
	_uriPath.clear();
	_version.clear();

	_hostAddress.clear();
	_contentLength = -1;
	_userAgent.clear();
	_connection.clear();
	_contentType.clear();
	_cookie.clear();
	_referer.clear();
	_body.clear();
}


// Getters

HttpRequest::Status HttpRequest::getStatus(void) const { return _status; }
const TStr& HttpRequest::getBuffer(void) const { return _buffer; }

HttpMethods::Type HttpRequest::getMethod(void) const { return _method; }
const TStr& HttpRequest::getUri(void) const { return _uriPath; }
const TStr& HttpRequest::getVersion(void) const { return _version; }
const TStr& HttpRequest::getHost(void) const { return _hostAddress; }
int			HttpRequest::getContentLength(void) const { return _contentLength; }
const TStr& HttpRequest::getUserAgent(void) const { return _userAgent; }
const TStr& HttpRequest::getConnection(void) const { return _connection; }
const TStr& HttpRequest::getContentType(void) const { return _contentType; }
const TStr& HttpRequest::getCookie(void) const { return _cookie; }
const TStr& HttpRequest::getReferer(void) const { return _referer; }

// Parsing buffer into an http request

void	HttpRequest::appendToBuffer(char recvBuffer[], size_t bytes)
{
	_buffer.append(recvBuffer, bytes);
}

bool	HttpRequest::tryParseHttpRequest(void)
{
	return (false);
}

// For testing
bool	HttpRequest::setValidRequestForTesting(void)
{
	// HttpRequest Logic
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

	// Request line
	_method = HttpMethods::GET;
	_uriPath = "/docs/";
	_version = "HTTP/1.1";

	// Headers - mandatory
	_hostAddress = "localhost";
	_contentLength = 0;

	// Headers - very common
	_userAgent = "Mozilla/5.0 (compatible; TestBot/1.0)";
    _connection = "keep-alive";
    _contentType = "";  // no body
    _cookie = "sessionid=abc123; csrftoken=xyz456";
    _referer = "http://localhost/";

	// Body
	_body.clear();
	
	return (true);
}

void	HttpRequest::setBuffer(const TStr& buffer) { _buffer = buffer; }

// Print
std::ostream&	HttpRequest::printRequest(std::ostream& o) const
{
	o	<< "Method : " << _method << '\n'
		<< "URI : " << _uriQuery << '\n'
		<< "Version : " << _version << '\n'
		<< "Host : " << _hostAddress << '\n'
		<< "Content Length : " << _contentLength << '\n'
		<< "User Agent : " << _userAgent << '\n'
		<< "Connection : " << _connection << '\n'
		<< "Content Type : " << _contentType << '\n'
		<< "Cookie : " << _cookie << '\n'
		<< "Refered : " << _referer << '\n'
		<< "Body : " << _body << std::endl;
	return (o);
}

