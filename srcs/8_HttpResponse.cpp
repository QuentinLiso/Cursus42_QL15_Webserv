/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   8_HttpResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 09:23:08 by qliso             #+#    #+#             */
/*   Updated: 2025/06/22 22:51:24 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "8_HttpResponse.hpp"

// 1 - Constructor & destructor

HttpResponse::HttpResponse(void) 
	: 	_responseStatusCode(100), 
		_responseBodyType(BODY_NONE), 
		_responseBodyStaticFd(-1),
		_errorOrigin(ERROR_NONE)
{}

HttpResponse::~HttpResponse(void) {}

// Static 
const char*	const* 	HttpResponse::httpStatusCodesReasons = HttpResponse::initHttpStatusCodes();

const char**		HttpResponse::initHttpStatusCodes(void)
{
	static const char*	array[512] = { NULL };

	array[100] = "Continue";
	array[101] = "Switching Protocols";
	array[102] = "Processing";
	array[103] = "Early Hints";
	array[200] = "OK";
	array[201] = "Created";
	array[202] = "Accepted";
	array[203] = "Non-Authoritative Information";
	array[204] = "No Content";
	array[205] = "Reset Content";
	array[206] = "Partial Content";
	array[207] = "Multi-Status";
	array[208] = "Already Reported";
	array[226] = "IM Used";
	array[300] = "Multiple Choices";
	array[301] = "Moved Permanently";
	array[302] = "Found";
	array[303] = "See Other";
	array[304] = "Not Modified";
	array[307] = "Temporary Redirect";
	array[308] = "Permanent Redirect";
	array[400] = "Bad Request";
	array[401] = "Unauthorized";
	array[402] = "Payment Required";
	array[403] = "Forbidden";
	array[404] = "Not Found";
	array[405] = "Method Not Allowed";
	array[406] = "Not Acceptable";
	array[407] = "Proxy Authentication Required";
	array[408] = "Request Timeout";
	array[409] = "Conflict";
	array[410] = "Gone";
	array[411] = "Length Required";
	array[412] = "Precondition Failed";
	array[413] = "Content Too Large";
	array[414] = "URI Too Long";
	array[415] = "Unsupported Media Type";
	array[416] = "Range Not Satisfiable";
	array[417] = "Expectation Failed";
	array[418] = "I'm a teapot";
	array[421] = "Misdirected Request";
	array[422] = "Unprocessable Content";
	array[423] = "Locked";
	array[424] = "Failed Dependency";
	array[425] = "Too Early";
	array[426] = "Upgrade Required";
	array[428] = "Precondition Required";
	array[429] = "Too Many Requests";
	array[431] = "Request Header Fields Too Large";
	array[451] = "Unavailable For Legal Reasons";
	array[500] = "Internal Server Error";
	array[501] = "Not Implemented";
	array[502] = "Bad Gateway";
	array[503] = "Service Unavailable";
	array[504] = "Gateway Timeout";
	array[505] = "HTTP Version Not Supported";
	array[506] = "Variant Also Negotiates";
	array[507] = "Insufficient Storage";
	array[508] = "Loop Detected";
	array[510] = "Not Extended";
	array[511] = "Network Authentication Required";
	
	return (array);
}

const char*			HttpResponse::getStatusCodeReason(unsigned short httpStatusCode)
{
	if (httpStatusCode < 100 || httpStatusCode > 511 || HttpResponse::httpStatusCodesReasons[httpStatusCode] == NULL)
		return (HttpResponse::httpStatusCodesReasons[500]);
	return (HttpResponse::httpStatusCodesReasons[httpStatusCode]);
}

std::map<TStr, TStr> 	HttpResponse::mimeMap = HttpResponse::initMimeMap();

std::map<TStr, TStr>	HttpResponse::initMimeMap(void)
{
	std::map<TStr, TStr>	mimeMap;
	
	mimeMap[".html"] = "text/html";
	mimeMap[".htm"] = "text/html";
	mimeMap[".css"] = "text/css";
	mimeMap[".js"] = "application/javascript";
	mimeMap[".png"] = "image/png";
	mimeMap[".jpg"] = "image/jpeg";
	mimeMap[".gif"] = "image/gif";
	mimeMap[".pdf"] = "application/pdf";
	mimeMap[".txt"] = "text/plain";

	return (mimeMap);
}

const TStr& 			HttpResponse::getMimeType(const TStr& filepath)
{
	static TStr	defaultMime("application/octet-stream");
	TStr	fileExtension = getFileExtension(filepath);

	std::map<TStr, TStr>::const_iterator	it = mimeMap.find(fileExtension);
	if (it != mimeMap.end())
		return (it->second);
	return (defaultMime);
}




// Private 
void	HttpResponse::setDefaultHeaders(void)
{
	_headers["Connection"] = "close";
	_headers["Server"] = "ft_webserv";

	char dateBuf[100];
    time_t now = time(0);
    struct tm gmt_date;
    gmtime_r(&now, &gmt_date);
    strftime(dateBuf, sizeof(dateBuf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_date);
    _headers["Date"] = TStr(dateBuf);
}

void	HttpResponse::setContentLengthHeader(int contentLength)
{ 
	_headers["Content-Length"] = convToStr(contentLength); 
}

void	HttpResponse::setContentTypeHeader(const TStr& filename)
{ 
	_headers["Content-Type"] = getMimeType(filename);
}

// Public 

void	HttpResponse::setDefaultErrorPage(int responseStatusCode, ErrorOrigin errorOrigin)
{
    std::ostringstream html;
    html << "<!DOCTYPE html>\n"
         << "<html lang=\"en\">\n"
         << "<head>\n"
         << "  <meta charset=\"UTF-8\">\n"
         << "  <title>" << responseStatusCode << " " << _reasonPhrase << "</title>\n"
         << "  <style>\n"
         << "    body {"
         << "      background-color: #f0f0f0;"
         << "      color: #000;"
         << "      font-family: sans-serif;"
         << "      text-align: center;"
         << "      padding-top: 10%;"
         << "    }\n"
         << "    h1 {"
         << "      font-size: 3em;"
         << "      margin-bottom: 0.5em;"
         << "    }\n"
         << "    p {"
         << "      color: #666;"
         << "    }\n"
         << "  </style>\n"
         << "</head>\n"
         << "<body>\n"
         << "  <h1>" << responseStatusCode << " " << _reasonPhrase << "</h1>\n"
         << "  <p>Made by ft_webserv.</p>\n"
         << "</body>\n"
         << "</html>\n";
	
	_responseStatusCode = responseStatusCode;
    _reasonPhrase = getStatusCodeReason(responseStatusCode);
	_responseBodyType = BODY_STRING;
	_responseBodyStr = html.str();
	setDefaultHeaders();
	setContentLengthHeader(_responseBodyStr.size());
	setContentTypeHeader(".html");
	_errorOrigin = errorOrigin;
}

void	HttpResponse::setCustomErrorPage(int responseStatusCode, const LocationConfig* locationConfig, ErrorOrigin errorOrigin)
{
	const std::map<ushort, TStr>& customErrorPages = locationConfig->getErrorPage().getErrorPages();
	std::map<ushort, TStr>::const_iterator it = customErrorPages.find(_responseStatusCode);
	
	if (it == customErrorPages.end())
		return (setDefaultErrorPage(responseStatusCode, errorOrigin));
	
	const TStr& filepath = it->second;
	struct stat	fileinfo;

	if (stat(filepath.c_str(), &fileinfo) != 0)
		return (setDefaultErrorPage(responseStatusCode, errorOrigin));


	if (!S_ISREG(fileinfo.st_mode))
		return (setDefaultErrorPage(responseStatusCode, errorOrigin));

	TStr	extension = getFileExtension(filepath);
	if (extension != ".html" && extension != ".htm")
		return (setDefaultErrorPage(responseStatusCode, errorOrigin));
	
	if (access(filepath.c_str(), R_OK) != 0)
		return (setDefaultErrorPage(responseStatusCode, errorOrigin));

	_responseBodyStaticFd = open(filepath.c_str(), O_RDONLY);
	if (_responseBodyStaticFd < 0)
		return (setDefaultErrorPage(responseStatusCode, errorOrigin));

	_responseStatusCode = responseStatusCode;
	_reasonPhrase = getStatusCodeReason(responseStatusCode);
	_responseBodyType = BODY_FILE_DESCRIPTOR;
	setDefaultHeaders();
	setContentLengthHeader(fileinfo.st_size);
	setContentTypeHeader(filepath);
	_errorOrigin = errorOrigin;
}

void	HttpResponse::setGetHeadStaticResponse(int responseStatusCode, const TStr& resolvedPath, const struct stat& resolvedPathStat, const LocationConfig* locationConfig, HttpMethods::Type method)
{
	if (method == HttpMethods::GET)
	{
		_responseBodyStaticFd = open(resolvedPath.c_str(), O_RDONLY);
		if (_responseBodyStaticFd < 0)
			return (setCustomErrorPage(500, locationConfig, ERROR_RESPONSE_BUILDING));
		Console::log(Console::INFO, "[SERVER] Sending file " + resolvedPath + " opened on FD " + convToStr(_responseBodyStaticFd) + " to client");
		_responseBodyType = BODY_FILE_DESCRIPTOR;
	}
	else
		_responseBodyType = BODY_NONE;

	_responseStatusCode = responseStatusCode;
	_reasonPhrase = getStatusCodeReason(responseStatusCode);
	setDefaultHeaders();
	setContentLengthHeader(resolvedPathStat.st_size);
	setContentTypeHeader(resolvedPath);
	
	const TStr mime = getMimeType(resolvedPath);
	if (mime == "application/pdf")
		_headers["Content-Disposition"] = "attachment";
}

void	HttpResponse::setGetHeadAutoindexResponse(int responseStatusCode, const TStr& resolvedPath, const LocationConfig* locationConfig, HttpMethods::Type method)
{
	DIR*	directory = opendir(resolvedPath.c_str());

	if (directory == NULL)
		return (setCustomErrorPage(500, locationConfig, ERROR_RESPONSE_BUILDING));

	std::ostringstream	html;
	html	<< "<!DOCTYPE html>\n"
			<< "<html lang=\"en\">\n"
			<< "<head>\n"
			<< "  <meta charset=\"UTF-8\">\n"
			<< "  <title> Index of " << resolvedPath << "</title>\n"
			<< "  <style>\n"
			<< "    body {"
			<< "      background-color: #f0f0f0;"
			<< "      color: #000;"
			<< "      font-family: sans-serif;"
			<< "      padding-top: 10%;"
			<< "    }\n"
			<< "    h1 {"
			<< "      font-size: 3em;"
			<< "      margin-bottom: 0.5em;"
			<< "    }\n"
			<< "    p {"
			<< "      color: #666;"
			<< "    }\n"
			<< "  </style>\n"
			<< "</head>\n"
			<< "<body>\n"
			<< "  <h1> Index of " << resolvedPath << "</h1><hr><pre>\n";

	struct dirent	*directoryEntry;
	while ((directoryEntry = readdir(directory)) != NULL)
	{
		TStr	name = directoryEntry->d_name;
		if (name == "." || name == "..")
			continue ;
		if (directoryEntry->d_type == DT_DIR)
			name += "/";
		html << "<a href=\"" << name << "\">" << name << "</a>\n";
	}

	html << "</pre><hr>"
	<< "<p>Made by ft_webserv.</p>\n"
	<< "</body>\n"
	<< "</html>\n";

	closedir(directory);

	_responseStatusCode = responseStatusCode;
	_reasonPhrase = getStatusCodeReason(responseStatusCode);

	if (method == HttpMethods::GET)
	{
		_responseBodyType = BODY_STRING;
		_responseBodyStr = html.str();
		setContentLengthHeader(_responseBodyStr.size());
	}
	else
	{
		_responseBodyType = BODY_NONE;
		setContentLengthHeader(html.str().size());
	}
	setDefaultHeaders();
	setContentTypeHeader(".html");
}

void	HttpResponse::setDeleteResponse(int responseStatusCode)
{
	_responseStatusCode = responseStatusCode;
	_reasonPhrase = getStatusCodeReason(responseStatusCode);
	_responseBodyType = BODY_NONE;
	setContentLengthHeader(0);
	setDefaultHeaders();
}

void	HttpResponse::setPutStaticResponse(int responseStatusCode)
{
	_responseStatusCode = responseStatusCode;
	_reasonPhrase = getStatusCodeReason(responseStatusCode);
	_responseBodyType = BODY_NONE;
	setContentLengthHeader(0);
	setDefaultHeaders();
}

void	HttpResponse::setCgiResponse(int responseStatusCode, const TStr& cgiOutputFilePath, int contentSize, const std::map<TStr, TStr>& cgiHeaders, const LocationConfig* locationConfig)
{
	_responseBodyStaticFd = open(cgiOutputFilePath.c_str(), O_RDONLY);
	Console::log(Console::INFO, "[SERVER] Preparing response to send file " + cgiOutputFilePath + " opened on FD " + convToStr(_responseBodyStaticFd) + " to client");
	
	if (_responseBodyStaticFd < 0)
		return (setCustomErrorPage(500, locationConfig, ERROR_RESPONSE_BUILDING));
	_responseStatusCode = responseStatusCode;
	_reasonPhrase = getStatusCodeReason(responseStatusCode);
	_responseBodyType = BODY_CGI;
	setDefaultHeaders();
	setContentLengthHeader(contentSize);
	for (std::map<TStr, TStr>::const_iterator it = cgiHeaders.begin(); it != cgiHeaders.end(); it++)
	{
		if (it->first == "Status" || it->first == "Content-length")
			continue ;
		_headers[it->first] = it->second;
	}
}


TStr HttpResponse::headersToString() const
{
    std::ostringstream response;
	
	// Set status line
    response << "HTTP/1.1 " << _responseStatusCode << " " << _reasonPhrase << "\r\n";

    // Set headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    response << "\r\n";
    return response.str();
}

HttpResponse::ResponseBodyType	HttpResponse::getResponseBodyType(void) const { return _responseBodyType; }
const TStr&			HttpResponse::getResponseBodyStr(void) const { return _responseBodyStr; }
int					HttpResponse::getResponseBodyFd(void) const { return _responseBodyStaticFd; }
