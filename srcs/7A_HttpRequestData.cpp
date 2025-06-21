/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7A_HttpRequestData.cpp                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:18 by qliso             #+#    #+#             */
/*   Updated: 2025/06/21 18:04:10 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "7A_HttpRequestData.hpp"

// Contructor & destructor

HttpRequestData::HttpRequestData(void) :
	_httpRequestDataStatusCode(100),
	_validHttpRequestData(true),
	_method(HttpMethods::UNKNOWN),
	_hostPort(0),
	_contentLength(0),
	_contentLengthSet(false),
	_connection(HttpRequestData::CONN_UNSET),
	_contentType(HttpRequestData::MEDIA_UNSET),
	_contentEncoding(HttpRequestData::CE_UNSET),
	_transferEncoding(HttpRequestData::TE_UNSET)
{}

HttpRequestData::~HttpRequestData(void) {}

//	Static variables

const size_t	HttpRequestData::_maxSizeRequestLine = 8192;
const size_t	HttpRequestData::_maxSizeUri = 4096;
const size_t	HttpRequestData::_maxHeaderCount = 100;
const size_t	HttpRequestData::_maxSizeHeaderLine = 8192;
const size_t	HttpRequestData::_maxSizeHeaderName = 256;
const size_t	HttpRequestData::_maxSizeHeaderValue = 8192 - 256;
const size_t	HttpRequestData::_maxSizeRequestAndHeaders = 8192 + 100 * 8192;

// HttpRequest - Parsing Request Line

bool	HttpRequestData::parseMethod(const TStr& subRequestLine)
{
	static std::map<TStr, HttpMethods::Type>	httpMethods;
	
	if (httpMethods.empty())
	{
		httpMethods["GET"] = HttpMethods::GET;
		httpMethods["POST"] = HttpMethods::POST;
		httpMethods["DELETE"] = HttpMethods::DELETE;
		httpMethods["PUT"] = HttpMethods::PUT;
		httpMethods["HEAD"] = HttpMethods::HEAD;
	}

	std::map<TStr, HttpMethods::Type>::const_iterator	it = httpMethods.find(subRequestLine);
	if (it == httpMethods.end())
		return (error(405, "Parsing method"));
	
	_method = it->second;
	return (true);
}

bool	HttpRequestData::parseUri(const TStr& subRequestLine)
{
	if (subRequestLine.size() > HttpRequestData::_maxSizeUri)
		return (error(413, "Parsing URI"));

	size_t	questionMark = subRequestLine.find('?');
	
	TStr	uriPath;
	TStr	uriQuery;

	if (questionMark == std::string::npos)
	{
		if (!checkUriEncoding(uriPath, subRequestLine, &isForbiddenRawByteUriPath, &isForbiddenDecodedByteUriPath))
			return (false);
		_uriPath = uriPath;
	}
	else if (questionMark == 0)
	{
		return (error(400, "Parse URI"));
	}
	else
	{
		if (!checkUriEncoding(uriPath, subRequestLine.substr(0, questionMark), &isForbiddenRawByteUriPath, &isForbiddenDecodedByteUriPath))
			return (false);
		if (!checkUriEncoding(uriQuery, subRequestLine.substr(questionMark + 1), &isForbiddenRawByteUriQuery, &isForbiddenDecodedByteUriQuery))
			return (false);
		_uriPath = uriPath;
		_uriQuery = uriQuery;
	}
	return (true);	
}

bool	HttpRequestData::isForbiddenRawByteUriPath(unsigned char c)
{
	static bool	table[256] = { false };
	static bool	initialized = false;

	if (!initialized)
	{
		for (size_t i = 0; i < 0x1F; i++)
			table[i] = true;	// NULL BYTE & Control chars
				
		table[0x20] = table[0x22] = table[0x23] = table[0x25] = table[0x3C] = table[0x3E] = table[0x3F] = 
		table[0x5B] = table[0x5C] = table[0x5D] = table[0x5E] = table[0x60] = table[0x7B] = table[0x7C] = 
		table[0x7D] = true;		// SPACE "#%<>?[\]^`{|}
		
		table[0x7F] = true;		// DEL

		for (size_t i = 128; i < 256; i++)
			table[i] = true;	// Non-ASCII

		initialized = true;
	}

	return (table[c]);
}

bool	HttpRequestData::isForbiddenDecodedByteUriPath(unsigned char c)
{
	static bool	table[256] = { false };
	static bool	initialized = false;

	if (!initialized)
	{
		for (size_t i = 0; i < 0x1F; i++)
			table[i] = true;	// NULL BYTE & Control chars
		
		table[0x22] = table[0x23] = table[0x25] = table[0x2F] = table[0x3C] = table[0x3E] = table[0x3F] = 
		table[0x5B] = table[0x5C] = table[0x5D] = table[0x5E] = table[0x60] = table[0x7B] = table[0x7C] = 
		table[0x7D] = true;// "#%/<>?[\]^`{|}
		
		table[0x7F] = true;		// DEL

		initialized = true;
	}

	return (table[c]);
}

bool	HttpRequestData::isForbiddenRawByteUriQuery(unsigned char c)
{
	static bool	table[256] = { false };
	static bool	initialized = false;

	if (!initialized)
	{
		for (size_t i = 0; i < 0x1F; i++)
			table[i] = true;	// NULL BYTE & Control chars
		
		table[0x20] = table[0x22] = table[0x23] = table[0x25] = table[0x3C] = table[0x3E] = 
		table[0x5B] = table[0x5C] = table[0x5D] = table[0x5E] = table[0x60] = table[0x7B] = table[0x7C] = 
		table[0x7D] = true;		// SPACE "#%<>[\]^`{|}
		
		table[0x7F] = true;		// DEL

		for (size_t i = 128; i < 256; i++)
			table[i] = true;	// Non-ASCII

		initialized = true;
	}

	return (table[c]);
}

bool	HttpRequestData::isForbiddenDecodedByteUriQuery(unsigned char c)
{
	static bool	table[256] = { false };
	static bool	initialized = false;

	if (!initialized)
	{
		for (size_t i = 0; i < 0x1F; i++)
			table[i] = true;	// NULL BYTE & Control chars
		
		table[0x7F] = true;		// DEL

		initialized = true;
	}

	return (table[c]);
}

bool	HttpRequestData::checkUriEncoding(TStr& decodedPath, const TStr& request, bool (*forbiddenRawByte)(unsigned char), bool (*forbiddenDecodedByte)(unsigned char))
{
	if (request.size() > 4096)
		return (error(414, "Uri encoding - uriPath too long"));
	
	decodedPath.reserve(request.size());
	
	for (size_t i = 0; i < request.size(); i++)
	{
		char c = request[i];
		if (c  == '%')
		{
			if (i + 2 >= request.size() || !std::isxdigit(request[i + 1]) || !std::isxdigit(request[i + 2]))
				return (error(400, "Uri encoding - invalid use of %-encoding (no %XX hex value)"));

			unsigned char	decodedChar = hexLiteralCharsToHexValueChar(request[i+1], request[i+2]);
			if (forbiddenDecodedByte(decodedChar))
				return (error(400, "Uri encoding - decoded char is not allowed in URI path"));

			decodedPath += decodedChar;
			i += 2;
		}
		else
		{
			if (forbiddenRawByte(static_cast<unsigned char>(c)))
			{
				return (error(400, "Uri encoding - invalid raw char"));
			}
				
			decodedPath += c;
		}
	}
	if (!isValidUtf8(decodedPath))
		return (false);

	return (true);
}

unsigned char	HttpRequestData::hexLiteralCharsToHexValueChar(unsigned char first, unsigned char second)
{
	unsigned char	hexFirst = hexLiteralCharToValueChar(first);		// value between 0 and 15 i.e 0000XXXX
	unsigned char	hexSecond = hexLiteralCharToValueChar(second);		// idem
	return ((hexFirst << 4 ) | hexSecond);								// 0000XXXX << 4 | 0000XXXX
}

unsigned char	HttpRequestData::hexLiteralCharToValueChar(unsigned char c)
{
	if (c >= '0' && c <= '9') return (c - '0');
	if (c >= 'a' && c <= 'f') return (c - 'a' + 10);
	if (c >= 'A' && c <= 'F') return (c - 'A' + 10);
	return (0xFF);
}

bool	HttpRequestData::isAllowedByte(unsigned char c, const unsigned char* forbiddenBytes)
{
	for (size_t i = 0; i < sizeof(forbiddenBytes); i++)
	{
		if (c == forbiddenBytes[i])
			return (false);
	}
	return (true);
}

bool	HttpRequestData::isValidUtf8(const TStr& decodedPath)
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

bool 	HttpRequestData::isValidContinuationByte(unsigned char c)
{
	return (c >= 0x80 && c <= 0xBF);
}

bool	HttpRequestData::parseVersion(const TStr& subRequestLine)
{
	if (subRequestLine != "HTTP/1.1" && subRequestLine != "HTTP/1.0")
		return (error(505, "Parsing version"));
	return (true);
}


bool	HttpRequestData::setHeaderHost(const TStr& headerValue)
{
	if (headerValue.empty())
		return (error(400, "Host header cannot be empty"));
	
	size_t	colon = headerValue.find(':');

	TStr	address;
	TStr	port;
	
	if (colon == 0 || colon == headerValue.size() - 1)
		return (error(400, "Invalid host header"));
	else if (colon != TStr::npos)		// colon ':' found
	{
		address = headerValue.substr(0, colon);
		if (!isValidHostAddress(address))
			return (false);

		port = headerValue.substr(colon + 1);
		unsigned short	portValue;
		if (strToVal<ushort>(port, portValue) == false || portValue == 0)
			return (error(400, "Invalid Host port value"));
		
		_hostAddress = address;
		_hostPort = portValue;
	}
	else		// no colon found
	{
		if (!isValidHostAddress(headerValue))
			return (false);
		_hostAddress = headerValue;
	}
	return (true);
}

bool	HttpRequestData::isValidHostAddress(const TStr& hostAddress)
{
	if ((!std::isalnum(hostAddress[0]) && hostAddress[0] != '.')
		|| (!std::isalnum(hostAddress[hostAddress.size() - 1]) && hostAddress[hostAddress.size() - 1] != '.'))
		return (error(400, "Invalid Host address character found"));
	for (size_t i = 1; i < hostAddress.size(); i++)
	{
		char c = hostAddress[i];
		if (!std::isalnum(c) && c != '-' && c != '.')
			return (error(400, "Invalid Host address character found"));
	}
	return (true);
}

bool	HttpRequestData::setContentLength(const TStr& headerValue)
{
	if (headerValue.empty())
		return (error(400, "Empty Content-Length header"));

	if (_contentLengthSet || _transferEncoding != TE_UNSET)
		return (error(400, "Request can only have at most either 1 header Content-Length or Transfer-Encoding"));

	unsigned int	contentLength;
	if (strToVal<unsigned int>(headerValue, contentLength) == false)
		return (error(400, "Content-Length header must be a positive integer"));
	
	_contentLength = contentLength;
	_contentLengthSet = true;
	return (true);
}

bool	HttpRequestData::setUserAgent(const TStr& headerValue)
{
	for (size_t i = 0; i < headerValue.size(); i++)
		if (headerValue[i] == 127 || headerValue[i] < 32)
			return (error(400, "Invalid char found in User-agent header"));
	
	if (!isValidUtf8(headerValue))
		return (error(400, "Invalid UTF-8 char found in User-agent header"));
	
	_userAgent = headerValue;
	return (true);
}

bool	HttpRequestData::setConnection(const TStr& headerValue)
{
	if (headerValue.empty())
		return (error(400, "Connection header cannot be empty"));
	
	TStr	keepAlive = "keep-alive";
	TStr	close = "close";
	size_t	len = headerValue.size();

	if (areCaseInsensitiveEquals(headerValue, "keep-alive"))
	{
		_connection = HttpRequestData::CONN_KEEP_ALIVE;
		return (true);
	}
	else if (areCaseInsensitiveEquals(headerValue, "close"))
	{
		_connection = HttpRequestData::CONN_CLOSE;
		return (true);
	}
	_connection = HttpRequestData::CONN_INVALID;
	return (error(400, "Connection header only accept 'keep-alive' or 'close'"));
}

bool	HttpRequestData::setContentType(const TStr& headerValue)
{
	static std::map<TStr, HttpRequestData::MediaType>	mediasMap;
	if (mediasMap.empty())
	{
		mediasMap["text/html"] = MEDIA_TEXT_HTML;
		mediasMap["text/plain"] = MEDIA_TEXT_PLAIN;
		mediasMap["application/x-www-form-urlencoded"] = MEDIA_APPLICATION_FORM_URLENCODED;
		mediasMap["multipart/form-data"] = MEDIA_MULTIPART_FORM_DATA;
	}

	if (headerValue.empty())
		return (error(400, "Content-Type header cannot be empty"));
	
	size_t	semicolon = headerValue.find(';');
	TStr	mediaType = headerValue.substr(0, semicolon);

	if (!isValidMediaType(mediaType))
		return (error(400, "Invalid media type in Content-Type header"));
	
	std::map<TStr, HttpRequestData::MediaType>::const_iterator it =  mediasMap.find(createLowercaseStr(mediaType));
	_contentType = it == mediasMap.end() ? MEDIA_INVALID : it->second;

	if (semicolon != TStr::npos)
	{
		TStr	mediaParams = headerValue.substr(semicolon + 1);
		if (!parseMediaParams(mediaParams))
			return (false);
	}
	else if (_contentType == MEDIA_MULTIPART_FORM_DATA)
		return (error(400, "Content-Type header multipart_form_data requires boundary argument"));

	return (true);		
}

bool	HttpRequestData::isValidMediaType(const TStr& mediaType)
{
	size_t	slash = mediaType.find('/');
	
	if (slash == 0 || slash == TStr::npos || slash == mediaType.size() - 1)
		return (false);
	for (size_t i = 0; i < slash; i++)
	{
		char c = mediaType[i];
		if (c < 33 || c == 127 || c == ';')
			return (false);
	}
	return (true);
}

bool	HttpRequestData::parseMediaParams(const TStr& mediaParams)
{
	TStrVect params = split(mediaParams, ";");

	if (params.empty())
	{
		if (_contentType == MEDIA_MULTIPART_FORM_DATA)
			return (error(400, "Content-Type header multipart_form_data requires boundary argument"));
		return (true);
	}

	if (_contentType == MEDIA_MULTIPART_FORM_DATA)
	{
		for (size_t i = 0; i < params.size(); i++)
		{
			TStr	param = trimHeadAndTail(params[i]);
			size_t	equal = param.find('=');
			
			if (equal == TStr::npos)
				continue ;
			
			if (createLowercaseStr(param.substr(0, equal)) == "boundary")
			{
				_multipartBoundary = param.substr(equal + 1);
				return (true);
			}
		}
		return (error(400, "Content-Type header multipart_form_data requires a boundary parameter"));
	}
	
	return (true);
}

bool	HttpRequestData::setCookie(const TStr& headerValue)
{
	if (headerValue.empty())
		return (true);

	TStrVect	cookies = split(headerValue, ";");

	for (size_t i = 0; i < cookies.size(); i++)
	{
		TStr	cookie = cookies[i];
		size_t	equal = cookie.find('=');
		if (equal == 0 || equal == TStr::npos || equal == cookie.size() - 1)
			continue ;
		
		TStr	cookieKey = trimHeadAndTail(cookie.substr(0, equal));
		TStr	cookieValue = trimHeadAndTail(cookie.substr(equal + 1));
		if (cookieKey.empty())
			continue ;

		if (!isValidCookie(cookieKey, cookieValue))
			continue ;
		
		_cookies.insert(std::make_pair(cookieKey, cookieValue));
	}

	return (true);
}

bool	HttpRequestData::isValidCookie(const TStr& cookieKey, const TStr& cookieValue)
{
	static const bool	allowedCookieKeyChar[128] = {
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
	true,  // ' - ASCII 39
	false, // ( - ASCII 40
	false, // ) - ASCII 41
	true,  // * - ASCII 42
	true,  // + - ASCII 43
	false,  // , - ASCII 44
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

	for (size_t i = 0; i < cookieKey.size(); i++)
	{
		unsigned char	c = static_cast<unsigned char>(cookieKey[i]);
		if (c > 127 || allowedCookieKeyChar[c] == false)
			return (false);
	}
	for (size_t i = 0; i < cookieValue.size(); i++)
	{
		char	c = cookieValue[i];
		if (c < 33 || c > 126 || c == ';')
			return (false);
	}
	return (true);
}

bool	HttpRequestData::setReferer(const TStr& headerValue)
{
	if (!headerValue.empty())
		_referer = headerValue;
	
	return (true);
}

bool	HttpRequestData::setContentEncoding(const TStr& headerValue)
{
	if (headerValue.empty() || areCaseInsensitiveEquals(headerValue, "identity"))
	{
		_contentEncoding = CE_IDENTITY;
		return (true);
	}
	_contentEncoding = CE_INVALID;
	return (error(415, "Unsupported Content-Encoding"));
}

bool	HttpRequestData::setTransferEncoding(const TStr& headerValue)
{
	if (headerValue.empty())
		return (true);
	if (_transferEncoding != HttpRequestData::TE_UNSET || _contentLengthSet)
		return (error(400, "Request can only have at most either 1 header Content-Length or Transfer-Encoding"));

	if (areCaseInsensitiveEquals(headerValue, "chunked"))
	{
		_transferEncoding = TE_CHUNKED;
		return (true);
	}
	_transferEncoding = TE_INVALID;
	return (error(501, "Transfer encoding type not implemented"));	
}


// Error
bool	HttpRequestData::error(unsigned short httpRequestDataStatusCode, const TStr& step)
{
	std::ostringstream	oss;
	oss << "Invalid request : " << step << " - Error code : " << httpRequestDataStatusCode;
	Console::log(Console::DEBUG, oss.str());
	
	_httpRequestDataStatusCode = httpRequestDataStatusCode;
	_validHttpRequestData = false;
	return (false);
}

// Setters
bool	HttpRequestData::setRequestLine(const TStr& method, const TStr& uri, const TStr& version)
{
	return (parseMethod(method) && parseUri(uri) && parseVersion(version));
}

bool	HttpRequestData::setHeaderField(const TStr& headerName, const TStr& headerValue)
{
	static const bool	allowedHeaderNameChar[128] = {
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

	static std::map<TStr, bool (HttpRequestData::*)(const TStr&)>	setHeaderFieldMap;
	if (setHeaderFieldMap.empty())
	{
		setHeaderFieldMap["host"] = &HttpRequestData::setHeaderHost;
		setHeaderFieldMap["content-length"] = &HttpRequestData::setContentLength;
		setHeaderFieldMap["user-agent"] = &HttpRequestData::setUserAgent;
		setHeaderFieldMap["connection"] = &HttpRequestData::setConnection;
		setHeaderFieldMap["content-type"] = &HttpRequestData::setContentType;
		setHeaderFieldMap["cookie"] = &HttpRequestData::setCookie;
		setHeaderFieldMap["referer"] = &HttpRequestData::setReferer;
		setHeaderFieldMap["content-encoding"] = &HttpRequestData::setContentEncoding;
		setHeaderFieldMap["transfer-encoding"] = &HttpRequestData::setTransferEncoding;
	}

	std::map<TStr, bool (HttpRequestData::*)(const TStr&)>::const_iterator	it = setHeaderFieldMap.find(headerName);
	if (it == setHeaderFieldMap.end())
	{
		for (size_t i = 0; i < headerName.size(); i++)
		{
			unsigned char	c = static_cast<unsigned char>(headerName[i]);
			if (c > 127 || allowedHeaderNameChar[c] == false)
				return (error(400, "Set header field, invalid char found in header name"));
		}
		for (size_t i = 0; i < headerValue.size(); i++)
			if (headerValue[i] > 126 || headerValue[i] < 32)
				return (error(400, "Set header field, invalid char found in header value"));
		return (true);
	}
	return ((this->*(it->second))(headerValue));
}

void	HttpRequestData::resetData(void)
{
	_httpRequestDataStatusCode = 100;
	_validHttpRequestData = true;
	
	_method = HttpMethods::UNKNOWN;
	_uriPath.clear();
	_uriQuery.clear();
	_version.clear();
	_hostAddress.clear();	
	_hostPort = 0;
	_contentLength = 0;
	_userAgent.clear();
	_connection = HttpRequestData::CONN_UNSET;
	_contentType = HttpRequestData::MEDIA_UNSET;
	_multipartBoundary.clear();
	_cookies.clear();
	_referer.clear();
	_contentEncoding = HttpRequestData::CE_UNSET;
	_transferEncoding = HttpRequestData::TE_UNSET;
	_body.clear();
}

void	HttpRequestData::setHttpStatusCode(unsigned short code) { _httpRequestDataStatusCode = code; }

// Getters
unsigned short		HttpRequestData::getHttpRequestDataStatusCode(void) const { return _httpRequestDataStatusCode; }
bool				HttpRequestData::getValidHttpRequestData(void) const { return _validHttpRequestData; }

HttpMethods::Type	HttpRequestData::getMethod(void) const { return _method; }
const TStr&			HttpRequestData::getUriPath(void) const { return _uriPath; }
const TStr&			HttpRequestData::getUriQuery(void) const { return _uriQuery; }
const TStr&			HttpRequestData::getVersion(void) const { return _version; }
const TStr&			HttpRequestData::getHostAddress(void) const { return _hostAddress; }
ushort				HttpRequestData::getHostPort(void) const { return _hostPort; }
uint				HttpRequestData::getContentLength(void) const { return _contentLength; }
bool				HttpRequestData::isContentLengthSet(void) const { return _contentLengthSet; }
const TStr&			HttpRequestData::getUserAgent(void) const { return _userAgent; }
HttpRequestData::ConnectionType			HttpRequestData::getConnection(void) const { return _connection; }
HttpRequestData::MediaType				HttpRequestData::getContentType(void) const { return _contentType; }
const TStr&			HttpRequestData::getMultipartBoundary(void) const { return _multipartBoundary; }
const std::map<TStr, TStr>&				HttpRequestData::getCookies(void) const { return _cookies; }
const TStr&			HttpRequestData::getReferer(void) const { return _referer; }
HttpRequestData::ContentEncodingType	HttpRequestData::getContentEncoding(void) const { return _contentEncoding; }
HttpRequestData::TransferEncodingType 	HttpRequestData::getTransferEncoding(void) const { return _transferEncoding; }
const TStr&			HttpRequestData::getBody(void) const { return _body; }

// Print
std::ostream&		HttpRequestData::printRequestData(std::ostream& o) const
{
		o 	<< "HttpRequestData status code : " << _httpRequestDataStatusCode << '\n'
			<< "HttpRequestData valid : " << _validHttpRequestData << '\n'

			<< "Method : " << _method << '\n'
			<< "URI path : " << _uriPath << '\n'
			<< "URI query : " << _uriQuery << '\n'
			<< "Version : " << _version << '\n'
			<< "Host address : " << _hostAddress << '\n'
			<< "Host Port : " << _hostPort << '\n'
			<< "Content Length : " << _contentLength << '\n'
			<< "User Agent : " << _userAgent << '\n'
			<< "Connection : " << _connection << '\n'
			<< "Content type : " << _contentType << '\n'
			<< "Multipart boundary : " << _multipartBoundary << '\n'
			// << "" << _cookies;
			<< "Referer : " << _referer << '\n'
			<< "Content encoding : " << _contentEncoding << '\n'
			<< "Transfer encoding : " << _transferEncoding << '\n'
			<< "Body : " << _body << '\n'
			<< std::endl;

	return (o);
}