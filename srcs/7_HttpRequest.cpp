/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7_HttpRequest.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:18 by qliso             #+#    #+#             */
/*   Updated: 2025/06/23 16:16:33 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "7_HttpRequest.hpp"

// Contructor & destructor
HttpRequest::HttpRequest() : 
	_requestState(PARSING_HEADERS_NEED_DATA),
	_index(0),
	_headersEndIndex(0),
	_headersSize(0),
	_headersCount(0),
	_maxBodySize(0),
	_requestBodySizeCount(0),
	_actualChunkedDataSize(0),
	_requestBodyType(REQUEST_BODY_NO_BODY),
	_requestBodyParsingFd(-1),
	_currentChunkSize(0)
{}

HttpRequest::~HttpRequest(void) { if (_requestBodyParsingFd > 0) close(_requestBodyParsingFd); }

// Static
uint	HttpRequest::_requestBodyParsingFdTmpCount = 0;

// HttpRequest - Parsing Headers
HttpRequest::RequestState	HttpRequest::findHeadersEnd(void)
{
	_headersEndIndex = _requestBuffer.find("\r\n\r\n");		
	
	if (_headersEndIndex == TStr::npos)		// CRLF CRLF marking headers end not found
	{	
		if (_requestBuffer.size() > HttpRequestData::_maxSizeRequestAndHeaders)
			return (error(413, "Headers too long", PARSING_HEADERS_INVALID)); // \r\n not found but max capacity reached
		_headersEndIndex = _requestBuffer.size() < 3 ? 0 : _requestBuffer.size() ;
		return (PARSING_HEADERS_NEED_DATA);
	}
	return (PARSING_HEADERS_END_FOUND);
}

HttpRequest::RequestState	HttpRequest::parseRequestLine(void)
{
	size_t	lineEnd = _requestBuffer.find("\r\n", _index);

	if (lineEnd > HttpRequestData::_maxSizeRequestLine)
		return (error(413, "Parsing request line", PARSING_HEADERS_INVALID));

	TStr	requestLine = _requestBuffer.substr(_index, lineEnd - _index);

	size_t	firstSpace = requestLine.find(' ', 0);
	if (firstSpace == 0 || firstSpace == std::string::npos)
		return (error(400, "Parsing request line", PARSING_HEADERS_INVALID));

	size_t	secondSpace = requestLine.find(' ', firstSpace + 1);
	if (secondSpace == (firstSpace + 1) || secondSpace == std::string::npos)
		return (error(400, "Parsing request line", PARSING_HEADERS_INVALID));

	if (requestLine.find(' ', secondSpace + 1) != std::string::npos)
		return (error(400, "Parsing request line", PARSING_HEADERS_INVALID));

	if (!_httpRequestData.setRequestLine(requestLine.substr(0, firstSpace), 
										requestLine.substr(firstSpace + 1, secondSpace - firstSpace - 1),
										requestLine.substr(secondSpace + 1)))
		return (error(_httpRequestData.getHttpRequestDataStatusCode(), "Parsing request line", PARSING_HEADERS_INVALID));

	_index = lineEnd + 2;
	return (PARSING_REQUEST_LINE_DONE);
}

HttpRequest::RequestState 	HttpRequest::parseHeaders(void)
{
	while (true)
	{
		size_t	lineEnd = _requestBuffer.find("\r\n", _index);		
		size_t	lineSize = lineEnd - _index;
		
		if (lineSize == 0)		// CRLF found on an empty line -> end of headers
		{
			_index += 2;
			return (PARSING_HEADERS_DONE);
		}

		if (lineSize > HttpRequestData::_maxSizeHeaderLine)
			return (error(431, "Parsing headers - Header size too long", PARSING_HEADERS_INVALID));

		_headersCount++;
		if (_headersCount > HttpRequestData::_maxHeaderCount)
			return (error(431, "Parsing headers - Too many headers", PARSING_HEADERS_INVALID));

		if (parseHeaderLine(_requestBuffer.substr(_index, lineSize)) == PARSING_HEADERS_INVALID)
			return (PARSING_HEADERS_INVALID);
		
		_headersSize = 0;
		_index = lineEnd + 2;
	}

	if (_httpRequestData.getHostAddress().empty())
			return (error(400, "Parsing headers - No host header", PARSING_HEADERS_INVALID));

}

HttpRequest::RequestState 	HttpRequest::parseHeaderLine(const TStr& headerLine)
{
	size_t	colon = headerLine.find(':');

	if (colon == 0 || colon == std::string::npos)
		return (error(400, "Parsing header line", PARSING_HEADERS_INVALID));
	if (colon >= HttpRequestData::_maxSizeHeaderName)
		return (error(431, "Parsing header line, header name too long", PARSING_HEADERS_INVALID));

	TStr	headerName;
	headerName.reserve(colon);
	for (size_t i = 0; i < colon; i++)
		headerName += std::tolower(headerLine[i]);
	TStr	headerValue = trimHeadAndTail(headerLine.substr(colon + 1));

	if (_httpRequestData.setHeaderField(headerName, headerValue) == false)
		return (error(_httpRequestData.getHttpRequestDataStatusCode(), "Parsing header line", PARSING_HEADERS_INVALID));
	return (PARSING_HEADERS_PROCESSING);
}

HttpRequest::RequestState	HttpRequest::setResponseBodyType(void)
{
	if (_httpRequestData.getTransferEncoding() == HttpRequestData::TE_CHUNKED)
		_requestBodyType = REQUEST_BODY_CHUNKED;

	else if (_httpRequestData.getContentLength() != 0)
		_requestBodyType = REQUEST_BODY_CONTENT_LENGTH;

	else
	{
		_requestBodyType = REQUEST_BODY_NO_BODY;
		if (!_requestBuffer.empty())
			return (error(400, "Client sent unexpected body", PARSING_HEADERS_INVALID));
	}

	return (PARSING_HEADERS_DONE);
}

// Prepare parsing body
HttpRequest::RequestState	HttpRequest::prepareParsingHttpRequestBody(size_t maxBodySize, bool putStaticRequest, const TStr& resolvedPath)
{
	_maxBodySize = maxBodySize;
	if (_httpRequestData.getContentLength() > _maxBodySize)		// Content-Length announced is greater that max body size -> error
		return (error(413, "Content Length of the body sent by client is greater that max body size", PARSING_BODY_INVALID));
	
 	if (_requestBuffer.size() > _maxBodySize)	// Buffer already larger than max body size or Content-Length (i.e client sent more bytes that announced)
		return (error(413, "Content Length of the body sent by client is greater that max body size", PARSING_BODY_INVALID));

 	// if (_requestBuffer.size() > _httpRequestData.getContentLength())	// Buffer already larger than max body size or Content-Length (i.e client sent more bytes that announced)
	// 	return (error(413, "Content Length of the body sent by client is greater that max body size", PARSING_BODY_DONE));
		

	if (putStaticRequest)
	{
		_requestBodyParsingFilepath = resolvedPath;
	}
	else 
	{
		std::ostringstream	tmpFilename;
		tmpFilename << "/home/qliso/Documents/Webserv_github/html/tmp/1_BodyRequest/tmp_file" << HttpRequest::_requestBodyParsingFdTmpCount++ << ".txt";
		_requestBodyParsingFilepath = tmpFilename.str();
	}

	_requestBodyParsingFd = open(_requestBodyParsingFilepath.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0600);
	if (_requestBodyParsingFd < 0)
		return (error(500, "Couldn't open file to store request body", PARSING_BODY_INVALID));
	Console::log(Console::INFO, "[SERVER] Opening file " + _requestBodyParsingFilepath + " on FD " + convToStr(_requestBodyParsingFd) + " to store request body");
	return (PARSING_BODY_FROM_BUFFER);	
	// At the end, we are sure that :
	// - Content Length announced is less that max body size (for content-lengh)
	// - Buffer has less or equal bytes than Content-Length (for content-length)
	// - Buffer has less or equal bytes than maxBodySize (for both content-length and chunked)
	// -> We can proceed to parsing the request buffer
}

HttpRequest::RequestState	HttpRequest::parseHttpBodyFromRequestBuffer(bool putStaticRequest)
{
	if (_requestBodyType == REQUEST_BODY_CONTENT_LENGTH)
	{
		if (_requestBuffer.empty())				// Buffer empty, we need recv to continue
			return (PARSING_BODY_CONTENT_LENGTH_NEED_DATA);
		
		ssize_t bytesWritten = 0;
		ssize_t	totalBytesWritten = 0;
		while (static_cast<size_t>(totalBytesWritten) < _requestBuffer.size())
		{
			Console::log(Console::WARNING, "Hello from parseHttpBodyFromRequestBuffer");
			bytesWritten = write(_requestBodyParsingFd, _requestBuffer.c_str() + totalBytesWritten, _requestBuffer.size() - totalBytesWritten);
			if (bytesWritten < 0)	// We failed to write all the bytes from the buffer to the fd
				return (error(500, "Failed to write request buffer to request body fd", PARSING_BODY_INVALID));
			totalBytesWritten += bytesWritten;
		}
		_requestBodySizeCount += totalBytesWritten;
		_requestBuffer.erase();
		if (totalBytesWritten == _httpRequestData.getContentLength())	// We wrote all the bytes, equal to buffer size, and the total is equal to content length
			return (PARSING_BODY_DONE);

		return (PARSING_BODY_CONTENT_LENGTH_NEED_DATA);	// We wrote all the bytes, equal to buffer size, and the total is less than content length -> need recv to continue
	}
	else if (_requestBodyType == REQUEST_BODY_CHUNKED)
	{
		_requestState = PARSING_CHUNK_SIZE;
		if (_requestBuffer.empty())	// Buffer empty, we need recv to continue
			return (PARSING_CHUNK_SIZE);
		return (parseChunkedHttpBodyFromRequestBuffer());
	}
	else
		return (PARSING_BODY_DONE);
}

bool						HttpRequest::setChunkSizeHexValue(const TStr& str, size_t& out)
{
	const char*	nptr = str.c_str();
	char*	endptr = NULL;

	out = std::strtoul(nptr, &endptr, 16);

	if (nptr == endptr)
		return (false);

	if ((out >= LONG_MAX))
		return (false);
	
	if (!endptr)
		return (false);

	if (!endptr || *endptr != '\r')
		return (false);
	
	return (true);
}

HttpRequest::RequestState	HttpRequest::parseChunkedHttpBodyFromRequestBuffer(void)
{
	size_t	lineEnd = 0;
	ssize_t bytesWritten = 0;
	ssize_t	totalBytesWritten = 0;

	while (true)
	{
		switch (_requestState)
		{
			case	PARSING_CHUNK_SIZE:
				lineEnd = _requestBuffer.find("\r\n");
				if (lineEnd == TStr::npos)	
					return (PARSING_CHUNK_SIZE);

				if (!setChunkSizeHexValue(_requestBuffer, _currentChunkSize))
					return (error(400, "Parsing chunk size didn't get a valid hex number", PARSING_BODY_INVALID));

				_requestBodySizeCount += lineEnd + 2;
				_requestBuffer.erase(0, lineEnd + 2);
				_requestState = _currentChunkSize == 0 ? PARSING_CHUNK_LAST_CRLF : PARSING_CHUNK_DATA;
				break ;
			
			case	PARSING_CHUNK_DATA:
				if (_requestBuffer.size() < _currentChunkSize + 2)
					return (PARSING_CHUNK_DATA);

				if (_requestBuffer[_currentChunkSize] != '\r' || _requestBuffer[_currentChunkSize + 1] != '\n')
					return (error(400, "Parsing data line size is not equal to the announced chunk size", PARSING_BODY_INVALID));

				while (static_cast<size_t>(totalBytesWritten) < _currentChunkSize)
				{
					bytesWritten = write(_requestBodyParsingFd, _requestBuffer.c_str() + totalBytesWritten, _currentChunkSize - totalBytesWritten);
					if (bytesWritten <= 0)	// We failed to write all the bytes from the buffer to the fd
						return (error(500, "Failed to write request buffer to request body fd", PARSING_BODY_INVALID));
					totalBytesWritten += bytesWritten;
				}

				_requestBodySizeCount += _currentChunkSize + 2;
				_requestBuffer.erase(0, _currentChunkSize + 2);
				_requestState = PARSING_CHUNK_SIZE;
				break ;

			case	PARSING_CHUNK_LAST_CRLF:
				if (_requestBuffer.size() < 2)
					return (PARSING_CHUNK_LAST_CRLF);
				if (_requestBuffer[0] != '\r' || _requestBuffer[1] != '\n')
					return (error(400, "Parsing data line size is not equal to the announced chunk size", PARSING_BODY_INVALID));
				
				_requestBodySizeCount += 2;
				_requestBuffer.erase(0, 2);
				if (!_requestBuffer.empty())
					return (error(400, "Buffer not empty after found last chunked CRLF", PARSING_BODY_INVALID));

				return (PARSING_BODY_DONE);	
			
			default : return (PARSING_BODY_DONE);
		}
	}
}

// Actually parsing body
HttpRequest::RequestState	HttpRequest::parseContentLengthBody(char recvBuffer[], size_t bytesReceived)
{
	Console::log(Console::DEBUG, "Hello from parseContentLength");
	_requestBodySizeCount += bytesReceived;

	if (_requestBodySizeCount > _httpRequestData.getContentLength())
		return (error(413, "Received too many bytes from request compared to announced Content-Length", PARSING_BODY_INVALID));

	ssize_t bytesWritten = 0;
	ssize_t	totalBytesWritten = 0;
	while (static_cast<size_t>(totalBytesWritten) < bytesReceived)
	{
		bytesWritten = write(_requestBodyParsingFd, recvBuffer + totalBytesWritten, bytesReceived - totalBytesWritten);
		if (bytesWritten < 0)	// We failed to write all the bytes from the buffer to the fd
			return (error(500, "Failed to write request buffer to request body fd", PARSING_BODY_INVALID));
		totalBytesWritten += bytesWritten;
	}

	if (_requestBodySizeCount == _httpRequestData.getContentLength())	// We wrote all the bytes, equal to buffer size, and the total is equal to content length
	{
		Console::log(Console::INFO, "All bytes received");
		return (PARSING_BODY_DONE);
	}

	return (PARSING_BODY_CONTENT_LENGTH_NEED_DATA);	// We wrote all the bytes, equal to buffer size, and the total is less than content length -> need recv to continue
}

HttpRequest::RequestState	HttpRequest::parseChunkedBody(char recvBuffer[], size_t bytesReceived)
{
	size_t	lineEnd;
	ssize_t bytesWritten = 0;
	ssize_t	totalBytesWritten = 0;

	_requestBodySizeCount += _requestBuffer.size() + bytesReceived;

	if (_requestBodySizeCount > _maxBodySize)
		return (error(413, "Received too many bytes from chunked body " + convToStr(_requestBodySizeCount) + " compared to max body size" + convToStr(_maxBodySize), PARSING_BODY_DONE));

	_requestBuffer.append(recvBuffer, bytesReceived);

	while (true)
	{
		switch (_requestState)
		{
			case	PARSING_CHUNK_SIZE:
				lineEnd = _requestBuffer.find("\r\n");
				if (lineEnd == TStr::npos)	
					return (PARSING_CHUNK_SIZE);

				if (!setChunkSizeHexValue(_requestBuffer, _currentChunkSize))
					return (error(400, "Parsing chunk size didn't get a valid hex number", PARSING_BODY_INVALID));

				_requestBodySizeCount += lineEnd + 2;
				_requestBuffer.erase(0, lineEnd + 2);
				_requestState = _currentChunkSize == 0 ? PARSING_CHUNK_LAST_CRLF : PARSING_CHUNK_DATA;
				break ;
			
			case	PARSING_CHUNK_DATA:
				if (_requestBuffer.size() < _currentChunkSize + 2)
					return (PARSING_CHUNK_DATA);

				if (_requestBuffer[_currentChunkSize] != '\r' || _requestBuffer[_currentChunkSize + 1] != '\n')
					return (error(400, "Parsing data line size is not equal to the announced chunk size", PARSING_BODY_INVALID));

				while (static_cast<size_t>(totalBytesWritten) < _currentChunkSize)
				{
					bytesWritten = write(_requestBodyParsingFd, _requestBuffer.c_str() + totalBytesWritten, _currentChunkSize - totalBytesWritten);
					if (bytesWritten <= 0)	// We failed to write all the bytes from the buffer to the fd
						return (error(500, "Failed to write request buffer to request body fd", PARSING_BODY_INVALID));
					totalBytesWritten += bytesWritten;
				}
				_actualChunkedDataSize += totalBytesWritten;
				_requestBodySizeCount += _currentChunkSize + 2;
				_requestBuffer.erase(0, _currentChunkSize + 2);
				_requestState = PARSING_CHUNK_SIZE;
				break ;

			case	PARSING_CHUNK_LAST_CRLF:
				if (_requestBuffer.size() < 2)
					return (PARSING_CHUNK_LAST_CRLF);
				if (_requestBuffer[0] != '\r' || _requestBuffer[1] != '\n')
					return (error(400, "Parsing data line size is not equal to the announced chunk size", PARSING_BODY_INVALID));
				
				_requestBodySizeCount += 2;
				_requestBuffer.erase(0, 2);
				if (!_requestBuffer.empty())
					return (error(400, "Buffer not empty after found last chunked CRLF", PARSING_BODY_INVALID));

				return (PARSING_BODY_DONE);	
			
			default : return (PARSING_BODY_DONE);
		}
	}
}


// Error
HttpRequest::RequestState	HttpRequest::error(unsigned short httpRequestStatusCode, const TStr& step, HttpRequest::RequestState requestState)
{
	std::ostringstream	oss;
	oss << "Invalid request : " << step << " - Error code : " << httpRequestStatusCode;
	Console::log(Console::DEBUG, oss.str());
	
	_requestState = requestState;
	_httpRequestData.setHttpStatusCode(httpRequestStatusCode);
	if (requestState == PARSING_BODY_INVALID)
	{
		Console::log(Console::INFO, "[SERVER] Closing file " + _requestBodyParsingFilepath + " on FD " + convToStr(_requestBodyParsingFd) + " storing request body");
		close (_requestBodyParsingFd);
		// unlink(_requestBodyParsingFilepath.c_str());
	}
	return (requestState);
}

void	HttpRequest::setMaxbodySize(size_t size) { _maxBodySize = size; }

HttpRequest::RequestState	HttpRequest::parseHttpRequest(char recvBuffer[], size_t bytesReceived)
{
	switch (_requestState)
	{
		case PARSING_HEADERS_NEED_DATA :
			_requestBuffer.append(recvBuffer, bytesReceived);
			
			_requestState = findHeadersEnd();
			if (_requestState != PARSING_HEADERS_END_FOUND) return (_requestState);
		
			_requestState = parseRequestLine();
			if (_requestState == PARSING_HEADERS_INVALID)	return (_requestState);
			
			_requestState = parseHeaders();
			if (_requestState == PARSING_HEADERS_INVALID)	return (_requestState);

			std::cout << "*** REQUEST HEADERS ***\n" << _requestBuffer.substr(0, _headersEndIndex + 4) << "********************" << std::endl;
			_requestBuffer.erase(0, _headersEndIndex + 4);
			_index = 0;

			_requestState = setResponseBodyType();
			if (_requestState == PARSING_HEADERS_INVALID)	return (_requestState);

			
			return (PARSING_HEADERS_DONE);
		
		default :	return (PARSING_HEADERS_DONE);
	}
}

HttpRequest::RequestState	HttpRequest::prepareParseHttpBody(const LocationConfig* locationConfig, bool putStaticRequest, const TStr& resolvedPath)
{
	if (_requestState == PARSING_HEADERS_DONE)
	{
		_requestState = prepareParsingHttpRequestBody(locationConfig->getClientMaxBodySize().getMaxBytes(), putStaticRequest, resolvedPath);
		if (_requestState == PARSING_BODY_INVALID)	
			return (PARSING_BODY_INVALID);
		
		_requestState = parseHttpBodyFromRequestBuffer(putStaticRequest);
		if (_requestState == PARSING_BODY_DONE)
		{
			Console::log(Console::INFO, "[SERVER] Closing file " + _requestBodyParsingFilepath + " on FD " + convToStr(_requestBodyParsingFd) + " storing request body");
			close (_requestBodyParsingFd);
		}
		return (_requestState);
	}
	return (PARSING_BODY_DONE);
}

HttpRequest::RequestState	HttpRequest::parseHttpBody(char recvBuffer[], size_t bytesReceived)
{
	if (_requestBodyType == REQUEST_BODY_CONTENT_LENGTH)
	{
		_requestState = parseContentLengthBody(recvBuffer, bytesReceived);
		if (_requestState == PARSING_BODY_DONE)
		{
			Console::log(Console::INFO, "[SERVER] Closing file " + _requestBodyParsingFilepath + " on FD " + convToStr(_requestBodyParsingFd) + " storing request body");
			close (_requestBodyParsingFd);
		}
		return (_requestState);
	}
	else
	{
		_requestState = parseChunkedBody(recvBuffer, bytesReceived);
		if (_requestState == PARSING_BODY_DONE)
		{
			Console::log(Console::INFO, "[SERVER] Closing file " + _requestBodyParsingFilepath + " on FD " + convToStr(_requestBodyParsingFd) + " storing request body");
			close (_requestBodyParsingFd);
		}
		return (_requestState);
	}
}


// Getters
HttpRequest::RequestState	HttpRequest::getHttpRequestState(void) const { return _requestState; }
const TStr&					HttpRequest::getRequestBuffer(void) const { return _requestBuffer; }
const HttpRequestData&		HttpRequest::getHttpRequestData(void) const { return _httpRequestData; }
size_t						HttpRequest::getRequestBodySize(void) const { return _requestBodySizeCount; }
size_t						HttpRequest::getActualChunkedDataSize(void) const { return _actualChunkedDataSize; }
const TStr&					HttpRequest::getRequestBodyParsingFilepath(void) const { return _requestBodyParsingFilepath; }

// For testing
void	HttpRequest::setBuffer(const TStr& buffer) { _requestBuffer = buffer; }


