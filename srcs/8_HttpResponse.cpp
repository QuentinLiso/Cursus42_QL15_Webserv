/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   8_HttpResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 09:23:08 by qliso             #+#    #+#             */
/*   Updated: 2025/06/06 11:14:21 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "8_HttpResponse.hpp"

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

void	HttpResponse::handleResolvedPath(const TStr& resolvedPath)
{
	int	status = stat(resolvedPath.c_str(), &_requestResolvedPathStatus);

	if (status != 0)
	{
		_statusCode = 404;
		return ;
	}

	if (S_ISDIR(_requestResolvedPathStatus.st_mode))
	{
		Console::log(Console::DEBUG, "Requested path is a directory");
		handleRequestedDirectory(resolvedPath);
	}
	if (S_ISREG(_requestResolvedPathStatus.st_mode))
	{
		Console::log(Console::DEBUG, "Requested path is a file");
		handleRequestedFile(resolvedPath);
	}
		
}

void	HttpResponse::handleRequestedDirectory(const TStr& resolvedPath)
{

}	

void	HttpResponse::handleRequestedFile(const TStr& resolvedPath)
{
	TStr	fileExtension = getFileExtension(resolvedPath);

	if (_locationConfig->getCgiExtensions().contains(fileExtension))
		std::cout << "Handling CGI file" << std::endl;
	else
		handleRequestedStaticFile(resolvedPath);
}

void	HttpResponse::handleRequestedStaticFile(const TStr& resolvedPath)
{
	if (access(resolvedPath.c_str(), R_OK) != 0)
	{
		_statusCode = 403;
		return ;
	}
	_bodyfd = open(resolvedPath.c_str(), O_RDONLY);
	if (_bodyfd < 0)
	{
		_statusCode = 500;
		return ;
	}
	_bodyType = HttpResponse::FILEDESCRIPTOR;
	_headers["Content-Length"] = convToStr(_requestResolvedPathStatus.st_size);
	_headers["Content-Type"] = getMimeType(resolvedPath);
	
	char lastModifBuf[100];
	struct tm gmt_lastModif;
	gmtime_r(&_requestResolvedPathStatus.st_mtim.tv_sec, &gmt_lastModif);
	strftime(lastModifBuf, sizeof(lastModifBuf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_lastModif);
	_headers["Last-Modified"] = TStr(lastModifBuf);
}

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


void	HttpResponse::handleErrorRequest(ushort statusCode)
{
	if (!canServeCustomErrorPage(statusCode))
	{
		_body = createDefaultStatusPage(statusCode);
		_bodyType = HttpResponse::STRING;
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


HttpResponse::HttpResponse(void) : _statusCode(200), _bodyType(HttpResponse::UNKNOWN), _body(), _bodyfd(-1) {}
HttpResponse::~HttpResponse(void) {}

HttpResponse::BodyType		HttpResponse::getBodyType(void) const { return _bodyType; }
int			HttpResponse::getBodyFd(void) const { return _bodyfd; }
const TStr&	HttpResponse::getBodyStr(void) const { return _body; };

void	HttpResponse::prepareResponse(const HttpRequest* httpRequest, const LocationConfig* locationConfig)
{
	_httpRequest = httpRequest;
	_locationConfig = locationConfig;

	_locationConfig->print(std::cout, 0);
	// Check HTTP method is allowed in this config
	if (!isRequestHttpMethodAllowed())
	{
		_statusCode = 405;
		return ;
	}

	// Build resolved filepath
	TStr	resolvedPath(_locationConfig->getFullPath() + _httpRequest->getUri().substr(_locationConfig->getLocationPath().getPath().size()));
	if (!isResolvedPathAllowed(resolvedPath))
	{
		_statusCode = 406;
		return ;
	}
	normalizeFilepath(resolvedPath);

	// Execute based on the resolved filepath
	handleResolvedPath(resolvedPath);

	if (_statusCode > 400 && _statusCode < 512)
		handleErrorRequest(_statusCode);

	// Set headers
	setDefaultHeaders();
}

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

void HttpResponse::print() const 
{
    std::cout << toString() << std::endl;
}





void HttpResponse::setErrorResponse(int code, const std::string& root, const std::string& errorPath) {
    setStatus(code);
    std::string fullPath = root + errorPath;
    if (!setBodyFromFile(fullPath)) {
        std::ostringstream ss;
        ss << "<h1>" << code << " " << getStatusCodeReason(code) << "</h1>";
        setBody(ss.str(), "text/html");
    }
}

void	HttpResponse::setStatus(int code) {
    _statusCode = code;
}

void	HttpResponse::addHeader(const TStr& key, const TStr& value) {
    _headers[key] = value;
}

void HttpResponse::setBody(const TStr& content, const TStr& contentType) {
    _body = content;
    std::ostringstream ss;
    ss << _body.size();
    _headers["Content-Length"] = ss.str();
    _headers["Content-Type"] = contentType;
}

bool HttpResponse::setBodyFromFile(const TStr& filePath)
{
    std::ifstream file(filePath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    file.close();

    TStr ext = filePath.substr(filePath.find_last_of(".") + 1);
    TStr contentType = "application/octet-stream";
    if (ext == "html" || ext == "htm") contentType = "text/html";
    else if (ext == "txt") contentType = "text/plain";
    else if (ext == "jpg" || ext == "jpeg") contentType = "image/jpeg";
    else if (ext == "png") contentType = "image/png";
    else if (ext == "css") contentType = "text/css";
    else if (ext == "js") contentType = "application/javascript";

    setBody(ss.str(), contentType);
    return true;
}
