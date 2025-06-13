/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   8_HttpResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 09:23:08 by qliso             #+#    #+#             */
/*   Updated: 2025/06/12 23:01:33 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "8_HttpResponse.hpp"

// 1 - Constructor & destructor

HttpResponse::HttpResponse(void) : _statusCode(200), _bodyType(HttpResponse::UNKNOWN), _body(), _bodyfd(-1) {}
HttpResponse::~HttpResponse(void) {}

// 2 - Static variables

const char*	const* HttpResponse::httpStatusCodes = HttpResponse::initHttpStatusCodes();

const char**	HttpResponse::initHttpStatusCodes(void)
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

const char*	HttpResponse::getStatusCodeReason(unsigned short httpStatusCode)
{
	if (httpStatusCode < 100 || httpStatusCode > 511 || HttpResponse::httpStatusCodes[httpStatusCode] == NULL)
		return (HttpResponse::httpStatusCodes[500]);
	return (HttpResponse::httpStatusCodes[httpStatusCode]);
}


std::map<TStr, TStr> HttpResponse::mimeMap = HttpResponse::initMimeMap();

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

const TStr& HttpResponse::getMimeType(const TStr& filepath)
{
	static TStr	defaultMime("application/octet-stream");
	TStr	fileExtension = getFileExtension(filepath);

	std::map<TStr, TStr>::const_iterator	it = mimeMap.find(fileExtension);
	if (it != mimeMap.end())
		return (it->second);
	return (defaultMime);
}

// 4 - Set Headers that will be present in all requests

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

// 5 - Handling bad requests (disallowed method + invalid request URI)

bool	HttpResponse::isRequestHttpMethodAllowed(void)
{
	bool	allowed = _locationConfig->getAllowedMethods().isAllowedMethod(_httpRequest->getMethod());
	if (!allowed)
		Console::log(Console::DEBUG, "HTTP request method not allowed in this location");
	return (allowed);
}

bool	HttpResponse::isResolvedPathAllowed(const TStr& resolvedPath)
{
	bool allowed = isValidFilepath(resolvedPath) && !containsDoubleDotsAccess(resolvedPath);
	if (!allowed)
		Console::log(Console::DEBUG, "HTTP request URI is an invalid filepath");
	return (allowed);
}

// 6 - Handling well formed requests

bool	HttpResponse::handleResolvedPath(const TStr& resolvedPath)
{
	int	status = stat(resolvedPath.c_str(), &_requestResolvedPathStatus);

	if (status != 0)
	{
		_statusCode = 404;
		return (false);
	}

	if (S_ISDIR(_requestResolvedPathStatus.st_mode))
	{
		Console::log(Console::DEBUG, "Requested path is a directory");
		return (handleRequestedDirectory(resolvedPath));
	}
	if (S_ISREG(_requestResolvedPathStatus.st_mode))
	{
		Console::log(Console::DEBUG, "Requested path is a file");
		return (handleRequestedFile(resolvedPath));
	}
	return (false);
}

// 7 - Handling a request for a directory

bool	HttpResponse::handleRequestedDirectory(const TStr& resolvedPath)
{
	// Checking index files
	if (_locationConfig->getIndex().isSet())
	{
		const TStrVect& indexFiles = _locationConfig->getIndex().getFullFileNames();
		for (size_t i = 0; i < indexFiles.size(); i++)
		{
			if (handleRequestedDirectoryIndexFile(indexFiles[i]))
				return (true);
		}
	}
	
	// Handle autoindex
	if (_locationConfig->getAutoindex().isActive())
	{
		return (handleRequestedDirectoryAutoindex(resolvedPath));
	}
		

	_statusCode = 403;
	return (false);
}	

bool	HttpResponse::handleRequestedDirectoryIndexFile(const TStr& filepath)
{
	TStr	fileExtension = getFileExtension(filepath);

	// Handling Index file CGI
	if (_locationConfig->getCgiExtensions().contains(fileExtension))
	{
		std::cout << "Handling CGI index file" << std::endl;
		return (true);
	}

	// Handling Index file static file
	else
	{
		struct stat	filestatus;
		if (stat(filepath.c_str(), &filestatus) != 0 || !S_ISREG(filestatus.st_mode) || access(filepath.c_str(), R_OK) != 0)
		{
			Console::log(Console::DEBUG, "Index file is not a readable static file");
			return (false);
		}
		_bodyfd = open(filepath.c_str(), O_RDONLY);
		if (_bodyfd < 0)
			return (false);
		
		_bodyType = HttpResponse::FILEDESCRIPTOR;
		_headers["Content-Length"] = convToStr(filestatus.st_size);
		_headers["Content-Type"] = getMimeType(filepath);
		
		char lastModifBuf[100];
		struct tm gmt_lastModif;
		gmtime_r(&filestatus.st_mtim.tv_sec, &gmt_lastModif);
		strftime(lastModifBuf, sizeof(lastModifBuf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_lastModif);
		_headers["Last-Modified"] = TStr(lastModifBuf);
				
		return (true);
	}
}

bool	HttpResponse::handleRequestedDirectoryAutoindex(const TStr& folderpath)
{
	DIR*	directory = opendir(folderpath.c_str());

	if (directory == NULL)
	{
		_statusCode = 500;
		return (false);
	}

	std::ostringstream	html;
	html << "<!DOCTYPE html>\n"
	<< "<html lang=\"en\">\n"
	<< "<head>\n"
	<< "  <meta charset=\"UTF-8\">\n"
	<< "  <title> Index of " << folderpath << "</title>\n"
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
	<< "  <h1> Index of " << folderpath << "</h1><hr><pre>\n";

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

	_statusCode = 200;
	_bodyType = HttpResponse::STRING;
	_body = html.str();
	_headers["Content-Length"] = convToStr(_body.size());
	_headers["Content-Type"] = "text/html";

	return (true);
}

// 8 - Handling a request for a static file

bool	HttpResponse::handleRequestedFile(const TStr& resolvedPath)
{
	TStr	fileExtension = getFileExtension(resolvedPath);

	if (_locationConfig->getCgiExtensions().contains(fileExtension))
	{
		std::cout << "Handling CGI file" << std::endl;
		return (true);
	}
		
	else
		return (handleRequestedStaticFile(resolvedPath));
}

bool	HttpResponse::handleRequestedStaticFile(const TStr& resolvedPath)
{
	if (access(resolvedPath.c_str(), R_OK) != 0)
	{
		_statusCode = 403;
		return (false);
	}
	_bodyfd = open(resolvedPath.c_str(), O_RDONLY);
	if (_bodyfd < 0)
	{
		_statusCode = 500;
		return (false);
	}
	_bodyType = HttpResponse::FILEDESCRIPTOR;
	_headers["Content-Length"] = convToStr(_requestResolvedPathStatus.st_size);
	_headers["Content-Type"] = getMimeType(resolvedPath);
	
	char lastModifBuf[100];
	struct tm gmt_lastModif;
	gmtime_r(&_requestResolvedPathStatus.st_mtim.tv_sec, &gmt_lastModif);
	strftime(lastModifBuf, sizeof(lastModifBuf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_lastModif);
	_headers["Last-Modified"] = TStr(lastModifBuf);
	return (true);
}

// 9 - Handling error requests

void	HttpResponse::handleErrorRequest(ushort statusCode)
{
	if (!canServeCustomErrorPage(statusCode))
	{
		_body = createDefaultStatusPage(statusCode);
		_bodyType = HttpResponse::STRING;
		_headers["Content-Length"] = convToStr(_body.size());
		_headers["Content-Type"] = "text/html";
	}
}

TStr	HttpResponse::createDefaultStatusPage(ushort statusCode)
{
    const char* reasonPhrase = HttpResponse::getStatusCodeReason(statusCode);
    std::ostringstream html;

    html << "<!DOCTYPE html>\n"
         << "<html lang=\"en\">\n"
         << "<head>\n"
         << "  <meta charset=\"UTF-8\">\n"
         << "  <title>" << statusCode << " " << reasonPhrase << "</title>\n"
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
         << "  <h1>" << statusCode << " " << reasonPhrase << "</h1>\n"
         << "  <p>Made by ft_webserv.</p>\n"
         << "</body>\n"
         << "</html>\n";

    return (html.str());
}

bool	HttpResponse::canServeCustomErrorPage(ushort statusCode)
{
	const std::map<ushort, TStr>& customErrorPages = _locationConfig->getErrorPage().getErrorPages();
	std::map<ushort, TStr>::const_iterator it = customErrorPages.find(statusCode);
	
	if (it == customErrorPages.end())
		return (false);
	
	const TStr& filepath = it->second;
	struct stat	fileinfo;

	if (stat(filepath.c_str(), &fileinfo) != 0)
		return (false);

	if (!S_ISREG(fileinfo.st_mode))
		return (false);

	TStr	extension = getFileExtension(filepath);
	if (extension != ".html" && extension != ".htm")
		return(false);
	
	if (access(filepath.c_str(), R_OK) != 0)
		return (false);

	_bodyfd = open(filepath.c_str(), O_RDONLY);
	if (_bodyfd < 0)
		return (false);

	_bodyType = HttpResponse::FILEDESCRIPTOR;
	_headers["Content-Length"] = convToStr(fileinfo.st_size);
	_headers["Content-Type"] = getMimeType(filepath);
	char lastModifBuf[100];
	struct tm gmt_lastModif;
	gmtime_r(&fileinfo.st_mtim.tv_sec, &gmt_lastModif);
	strftime(lastModifBuf, sizeof(lastModifBuf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_lastModif);
	_headers["Last-Modified"] = TStr(lastModifBuf);
	return (true);
}

// 10 - Getters
HttpResponse::BodyType		HttpResponse::getBodyType(void) const { return _bodyType; }
int			HttpResponse::getBodyFd(void) const { return _bodyfd; }
const TStr&	HttpResponse::getBodyStr(void) const { return _body; };


// 11 - Set HttpResponse fields from request
void	HttpResponse::prepareResponse(const HttpRequest* httpRequest, const LocationConfig* locationConfig)
{
	_httpRequest = httpRequest;
	_locationConfig = locationConfig;

	// _locationConfig->print(std::cout, 0);
	
	// Add default headers used for any Http Response
	setDefaultHeaders();

	if (_httpRequest->getStatus() == HttpRequest::INVALID)
	{
		_statusCode = _httpRequest->getStatusCode();
		handleErrorRequest(_statusCode);
		return ;
	}

	// Check HTTP method is allowed in this config
	if (!isRequestHttpMethodAllowed())
	{
		_statusCode = 405;
		handleErrorRequest(_statusCode);
		return ;
	}

	// Build resolved filepath
	TStr	resolvedPath(_locationConfig->getFullPath() + _httpRequest->getUriPath().substr(_locationConfig->getLocationPath().getPath().size()));
	if (!isResolvedPathAllowed(resolvedPath))
	{
		_statusCode = 406;
		handleErrorRequest(_statusCode);
		return ;
	}
	normalizeFilepath(resolvedPath);

	// Execute based on the resolved filepath
	if (!handleResolvedPath(resolvedPath))
	{
		handleErrorRequest(_statusCode);
		return ;
	}
}

void	HttpResponse::prepareErrorResponse(unsigned short code)
{
	_body = createDefaultStatusPage(code);
	_bodyType = HttpResponse::STRING;
	_headers["Content-Length"] = convToStr(_body.size());
	_headers["Content-Type"] = "text/html";
}

// 12 - Convert HttpResponse fields to a string to send
TStr HttpResponse::toString() const
{
    std::ostringstream response;
	
	// Set status line
    response << "HTTP/1.1 " << _statusCode << " " << getStatusCodeReason(_statusCode) << "\r\n";

    // Set headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it) {
        response << it->first << ": " << it->second << "\r\n";
    }
    response << "\r\n";
	
    return response.str();
}


// 13 - Debug printing
void HttpResponse::print() const 
{
    std::cout << toString() << std::endl;
}

