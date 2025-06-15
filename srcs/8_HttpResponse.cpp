/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   8_HttpResponse.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/04 09:23:08 by qliso             #+#    #+#             */
/*   Updated: 2025/06/15 17:31:26 by qliso            ###   ########.fr       */
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

TStr	HttpResponse::buildResolvedPath(void)
{
	const Root&	root = _locationConfig->getRoot();
	if (root.isSet())
	{
		return (root.getFolderPath() + _httpRequest->getUriPath());
	}
	else
	{
		const TStr& uri = _httpRequest->getUriPath();
		const TStr& aliasPath = _locationConfig->getAlias().getFolderPath();
		const TStr& locationPath = _locationConfig->getLocationPath().getPath();
		return (aliasPath + uri.substr(locationPath.size()));
	}
}

bool	HttpResponse::isResolvedPathAllowed(const TStr& resolvedPath)
{
	bool allowed = isValidFilepath(resolvedPath) && !containsDoubleDotsAccess(resolvedPath);
	if (!allowed)
		Console::log(Console::DEBUG, "HTTP request URI is an invalid filepath");
	return (allowed);
}


// GET + HEAD
HttpResponse::Status	HttpResponse::prepareResponseToGetFromHeaders(const TStr& resolvedPath)
{
	_requestResolvedPathStatResult = stat(resolvedPath.c_str(), &_requestResolvedPathStatus);
	
	if (_requestResolvedPathStatResult != 0)
		return (handleErrorRequest(404));
			
	if (S_ISDIR(_requestResolvedPathStatus.st_mode))
	{
		Console::log(Console::DEBUG, "Requested path is a directory");
		return (handleGetDirectory(resolvedPath));
	}
	else if (S_ISREG(_requestResolvedPathStatus.st_mode))
	{
		Console::log(Console::DEBUG, "Requested path is a file");
		return (handleGetFile(resolvedPath));
	}
	return (handleErrorRequest(404));
}

HttpResponse::Status	HttpResponse::handleGetDirectory(const TStr& resolvedPath)
{
	// Handle autoindex
	if (_locationConfig->getAutoindex().isActive())
	{
		return (handleGetDirectoryAutoindex(resolvedPath));
	}

	// Checking index files
	if (_locationConfig->getIndex().isSet())
	{
		const TStrVect& indexFiles = _locationConfig->getIndex().getFileNames();
		for (size_t i = 0; i < indexFiles.size(); i++)
		{
			if (handleGetDirectoryIndex(resolvedPath + '/' + indexFiles[i]) == 200)
				return (READY_TO_SEND);
		}
	}

	return (handleErrorRequest(404));
}

unsigned short			HttpResponse::handleGetDirectoryIndex(const TStr& resolvedPath)
{
	if (_locationConfig->getCgiExtensions().contains(getFileExtension(resolvedPath)))
		return (handleGetCgiFile(resolvedPath));
	else
		return (handleGetStaticFile(resolvedPath));
}

HttpResponse::Status	HttpResponse::handleGetDirectoryAutoindex(const TStr& resolvedPath)
{
	DIR*	directory = opendir(resolvedPath.c_str());

	if (directory == NULL)
		return (handleErrorRequest(500));

	std::ostringstream	html;
	html << "<!DOCTYPE html>\n"
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

	_statusCode = 200;
	_bodyType = HttpResponse::STRING;
	_body = html.str();
	_headers["Content-Length"] = convToStr(_body.size());
	_headers["Content-Type"] = "text/html";

	return (READY_TO_SEND);
}

HttpResponse::Status	HttpResponse::handleGetFile(const TStr& resolvedPath)
{
	if (_locationConfig->getCgiExtensions().contains(getFileExtension(resolvedPath)))
		_statusCode = handleGetCgiFile(resolvedPath);
	else
		_statusCode = handleGetStaticFile(resolvedPath);
	
	if (_statusCode != 200)
		return (handleErrorRequest(_statusCode));
	return (READY_TO_SEND);
}

unsigned short			HttpResponse::handleGetCgiFile(const TStr& resolvedPath)
{
	Console::log(Console::DEBUG, "Handling CGI GET request");
	int	outputPipe[2];

	if (pipe(outputPipe) == -1)
		return (500);
	
	pid_t	pid = fork();
	if (pid < 0)
	{
		close(outputPipe[0]); close(outputPipe[1]);
		return (500);
	}
	else if (pid == 0)
	{
		close(outputPipe[0]);
		if (dup2(outputPipe[1], STDOUT_FILENO) == -1)
		{
			close(outputPipe[1]);
			exit (1);
		}
		close(outputPipe[1]);
		TStrVect			tmpEnvp;
		std::vector<char*>	argv, envp;
		buildExecveArgs(resolvedPath, tmpEnvp, argv, envp);
		for (size_t i = 0; i < envp.size(); i++)
		{
			std::cout << envp[i] << std::endl;
		}
		if (chdir(getParentDirectory(resolvedPath).c_str()) == -1)
			exit (1);
		execve(argv[0], argv.data(), envp.data());
		Console::log(Console::ERROR, strerror(errno));
		exit(1);
		return (500);
	}
	else
	{
		close(outputPipe[1]);

		char	buffer[4096];
		ssize_t	bytesRead;
		TStr	cgiOutput;
		while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
			cgiOutput.append(buffer, bytesRead);

		close(outputPipe[0]);
		int status;
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status))
		{
			Console::log(Console::ERROR, "CGI process terminated abnormally");
			return (500);
		}
		else if (WEXITSTATUS(status) != 0)
		{
			Console::log(Console::ERROR, "CGI exited with non-zero status");
			return (500);
		}
		return (prepareResponseFromCgi(resolvedPath, cgiOutput));
	}
}

unsigned short			HttpResponse::handleGetStaticFile(const TStr& resolvedPath)
{
	if (stat(resolvedPath.c_str(), &_requestResolvedPathStatus) != 0)
		return (404);

	if (access(resolvedPath.c_str(), R_OK) != 0)
		return (403);

	_headers["Content-Length"] = convToStr(_requestResolvedPathStatus.st_size);
	_headers["Content-Type"] = getMimeType(resolvedPath);
	
	char lastModifBuf[100];
	struct tm gmt_lastModif;
	gmtime_r(&_requestResolvedPathStatus.st_mtim.tv_sec, &gmt_lastModif);
	strftime(lastModifBuf, sizeof(lastModifBuf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_lastModif);
	_headers["Last-Modified"] = TStr(lastModifBuf);

	if (_httpRequest->getMethod() == HttpMethods::HEAD)
	{
		_bodyType = NO_BODY;
		return (200);
	}

	_bodyfd = open(resolvedPath.c_str(), O_RDONLY);
	if (_bodyfd < 0)
		return (500);
	_bodyType = HttpResponse::FILEDESCRIPTOR;
	return (200);
}

// DELETE
HttpResponse::Status	HttpResponse::prepareResponseToDeleteFromHeaders(const TStr& resolvedPath)
{
	int	status = stat(resolvedPath.c_str(), &_requestResolvedPathStatus);

	if (status != 0)
		return (handleErrorRequest(404));

	TStr	parentDir = getParentDirectory(resolvedPath);

	if (access(parentDir.c_str(), X_OK | W_OK) != 0)
			return (handleErrorRequest(403));

	if (S_ISDIR(_requestResolvedPathStatus.st_mode))
	{
		_statusCode = isDirectoryEmpty(resolvedPath);
		if (_statusCode != 204)
			return (handleErrorRequest(_statusCode));
		
		if (rmdir(resolvedPath.c_str()) != 0)
			return (handleErrorRequest(500));
	}
	else if (S_ISREG(_requestResolvedPathStatus.st_mode))
	{
		if (unlink(resolvedPath.c_str()) != 0)
			return (handleErrorRequest(500));
		
		_statusCode = 204;
	}
	else
		return (handleErrorRequest(404));
	
	_bodyType = NO_BODY;
	return (READY_TO_SEND);
}

TStr			HttpResponse::getParentDirectory(const TStr& resolvedPath)
{
	if (resolvedPath.empty() || resolvedPath.size() == 1)
		return ("/");

	bool trailingSlash = (resolvedPath[resolvedPath.size() - 1] == '/');
	size_t	pos = trailingSlash ? 
			resolvedPath.find_last_of('/', resolvedPath.size() - 2) :
			resolvedPath.find_last_of('/');

	return (pos == TStr::npos ? "/" : resolvedPath.substr(0, pos));
}

unsigned short	HttpResponse::isDirectoryEmpty(const TStr& resolvedPath)
{
	DIR*	directory = opendir(resolvedPath.c_str());

	if (directory == NULL)
		return (500);

	struct dirent	*directoryEntry;
	while ((directoryEntry = readdir(directory)) != NULL)
	{
		TStr	name = directoryEntry->d_name;
		if (name != "." && name != "..")
		{
			closedir(directory);
			return (409);
		}
	}
	closedir(directory);
	return (204);
}	

// PUT
HttpResponse::Status	HttpResponse::prepareResponseToPutFromHeaders(const TStr& resolvedPath)
{
	_requestResolvedPathStatResult = stat(resolvedPath.c_str(), &_requestResolvedPathStatus);
	
	TStr	parentDir = getParentDirectory(resolvedPath);

	if (_requestResolvedPathStatResult != 0)	// File or folder does not exist
	{
		if (access(parentDir.c_str(), W_OK | X_OK) != 0)
			return (handleErrorRequest(403));
	}
	else
	{
		if (S_ISDIR(_requestResolvedPathStatus.st_mode)) // Existing URI folder
			return (handleErrorRequest(405));
		else if (!S_ISREG(_requestResolvedPathStatus.st_mode) || access(resolvedPath.c_str(), W_OK) != 0)	// Existing file not accessible or not a file
			return (handleErrorRequest(403));
	}

	if (_httpRequest->getTransferEncoding() != HttpRequest::TE_CHUNKED && _httpRequest->getContentLength() == 0)
		return (handleErrorRequest(411));

	return (PROCESSING);
}

// POST
HttpResponse::Status	HttpResponse::prepareResponseToPostFromHeaders(const TStr& resolvedPath)
{
	_requestResolvedPathStatResult = stat(resolvedPath.c_str(), &_requestResolvedPathStatus);
	
	TStr	parentDir = getParentDirectory(resolvedPath);
	bool	isCgiUri = _locationConfig->getCgiExtensions().contains(getFileExtension(resolvedPath));

	if (_requestResolvedPathStatResult != 0)	// Non-existing URI
	{
		if (!isCgiUri)							// Non-existing non-cgi URI
			return (handleErrorRequest(404));
			
		if (access(parentDir.c_str(), W_OK | X_OK) != 0)	// Parent folder not writeable executable
			return (handleErrorRequest(403));
		
	}
	else	// Existing URI
	{
		if (S_ISDIR(_requestResolvedPathStatus.st_mode)) // Existing URI folder
			return (handleErrorRequest(405));
		else if (S_ISREG(_requestResolvedPathStatus.st_mode))	// Existing URI file
		{
			if (!isCgiUri)		// Existing non-cgi URI 
				return (handleErrorRequest(405));

			if (access(resolvedPath.c_str(), W_OK) != 0) // File not writeable
				return (handleErrorRequest(403));
		} 
		else
			return (handleErrorRequest(403));
	}

	if (_httpRequest->getTransferEncoding() != HttpRequest::TE_CHUNKED && _httpRequest->getContentLength() == 0)
		return (handleErrorRequest(411));

	return (PROCESSING);
}

HttpResponse::Status	HttpResponse::prepareResponseToPostFromBody(const TStr& resolvedPath)
{
	// Request is necessarily a CGI
	Console::log(Console::DEBUG, "Handling CGI POST request");
	int	inputPipe[2];
	int	outputPipe[2];

	// Pipe and fork setup + check
	if (pipe(inputPipe) == -1)
		return (handleErrorRequest(500));
	if (pipe(outputPipe) == -1)
	{
		close(inputPipe[0]); close(inputPipe[1]);
		return (handleErrorRequest(500));
	}
	pid_t	pid = fork();
	if (pid < 0)
	{
		close(inputPipe[0]); close(inputPipe[1]); close(outputPipe[0]); close(outputPipe[1]);
		return (handleErrorRequest(500));
	}
	
	// Child process
	else if (pid == 0)
	{
		// Pipe and Dup setup -> Child process reads from inputPipe (waiting phase) and writes script execve output to outputPipe
		close(inputPipe[1]); close(outputPipe[0]);	// Closing writing-end of input, closing reading-end of output (not used)
		if (dup2(inputPipe[0], STDIN_FILENO) == -1 || dup2(outputPipe[1], STDOUT_FILENO) == -1)
		{
			close(inputPipe[0]); close(outputPipe[1]);
			exit (1);
		}
		close(inputPipe[0]); close(outputPipe[1]);	// Closing reading-end of input, closing writing-end of output (duped with stdin/out)

		// Exec
		TStrVect			tmpEnvp;
		std::vector<char*>	argv, envp;
		buildExecveArgs(resolvedPath, tmpEnvp, argv, envp);
		if (chdir(getParentDirectory(resolvedPath).c_str()) == -1)
			exit (1);
		execve(argv[0], argv.data(), envp.data());
		Console::log(Console::ERROR, strerror(errno));
		exit(1);
		return (handleErrorRequest(500));
	}

	// Parent process
	else
	{
		close(inputPipe[0]); close(outputPipe[1]);	// Closing reading-end of input, closing writing-end of output
		
		// Parent writing request body to inputPipe
		ssize_t bytesWritten = 0;
		const char*	requestBody = _httpRequest->getBody().c_str();
		ssize_t	bodySize = static_cast<ssize_t>(_httpRequest->getBody().size());
		
		while (bytesWritten < bodySize)
		{
			ssize_t	res = write(inputPipe[1], requestBody + bytesWritten, bodySize - bytesWritten);
			if (res <= 0)
			{
				close (inputPipe[1]); close (outputPipe[0]);
				waitpid(pid, NULL, 0);
				return (handleErrorRequest(500));
			}
			bytesWritten += res;
		}
		close(inputPipe[1]);
		
		// Parent reading child script execution from outputPipe
		Console::log(Console::DEBUG, "Handling parent CGI POST request");
		
		char	buffer[4096];
		ssize_t	bytesRead;
		TStr	cgiOutput;
		while ((bytesRead = read(outputPipe[0], buffer, sizeof(buffer))) > 0)
			cgiOutput.append(buffer, bytesRead);

		close(outputPipe[0]);
		int status;
		waitpid(pid, &status, 0);
		if (!WIFEXITED(status))
		{
			Console::log(Console::ERROR, "CGI process terminated abnormally");
			return (handleErrorRequest(500));
		}
		else if (WEXITSTATUS(status) != 0)
		{
			Console::log(Console::ERROR, "CGI exited with non-zero status");
			return (handleErrorRequest(500));
		}
		_statusCode = prepareResponseFromCgi(resolvedPath, cgiOutput);
		if (_statusCode >= 400)
			return (handleErrorRequest(_statusCode));
	}
	return (READY_TO_SEND);
}


// CGI
void	HttpResponse::buildExecveArgs(const TStr& resolvedPath, TStrVect& tmpEnvp, std::vector<char*>& argv, std::vector<char*>& envp)
{
	argv.push_back(const_cast<char*>(_locationConfig->getCgiPass().getExecPath().c_str()));
	argv.push_back(const_cast<char *>(resolvedPath.c_str()));
	argv.push_back(NULL);

	tmpEnvp.push_back("REQUEST_METHOD=" + HttpMethods::toString(_httpRequest->getMethod()));
	tmpEnvp.push_back("SCRIPT_NAME=" + resolvedPath);
	tmpEnvp.push_back("PATH_INFO=" + resolvedPath);
	tmpEnvp.push_back("QUERY_STRING=" + _httpRequest->getUriQuery());
	tmpEnvp.push_back("GATEWAY_INTERFACE=CGI/1.1");
	tmpEnvp.push_back("SERVER_PROTOCOL=HTTP/1.1");
	tmpEnvp.push_back("CONTENT_LENGTH=" + convToStr(_httpRequest->getBody().size()));

	envp.reserve(tmpEnvp.size() + 1);
	for (size_t i = 0; i < tmpEnvp.size(); i++)
		envp.push_back(const_cast<char*>(tmpEnvp[i].c_str()));
	envp.push_back(NULL);
}

unsigned short	HttpResponse::prepareResponseFromCgi(const TStr& resolvedPath, const TStr& cgiOutput)
{
	size_t	headersEnd = cgiOutput.find("\r\n\r\n");
	if (headersEnd == TStr::npos)
		return (502);

	std::istringstream	iss(cgiOutput.substr(0, headersEnd));
	TStr	line;

	while (std::getline(iss, line) && !line.empty())
	{
		if (line[line.size() - 1] == '\r')
			line.erase(line.size() - 1);
		size_t	colon = line.find(':');
		if (colon == 0 || colon == TStr::npos || colon == line.size() - 1)
			continue ;
		TStr	key = trimHeadAndTail(line.substr(0, colon));
		TStr	value = trimHeadAndTail(line.substr(colon + 1));
		_headers[key] = value;
	}

	if (_headers.find("Content-Length") == _headers.end())
		_headers["Content-Length"] = convToStr(cgiOutput.size() - (headersEnd + 4));
	
	if (_headers.find("Content-Type") == _headers.end())
		_headers["Content-Type"] = getMimeType(resolvedPath);

	char lastModifBuf[100];
	struct tm gmt_lastModif;
	gmtime_r(&_requestResolvedPathStatus.st_mtim.tv_sec, &gmt_lastModif);
	strftime(lastModifBuf, sizeof(lastModifBuf), "%a, %d %b %Y %H:%M:%S GMT", &gmt_lastModif);
	_headers["Last-Modified"] = TStr(lastModifBuf);
	
	std::map<TStr, TStr>::iterator it = _headers.find("Status");
	if (it != _headers.end())
	{
		if (strToVal<unsigned short>(it->second, _statusCode) == false)
			_statusCode = 200;
		_headers.erase("Status");
	}
	else
		_statusCode = 200;
	_bodyType = HttpResponse::STRING;
	_body = cgiOutput.substr(headersEnd + 4);
	return (_statusCode);
}

// Handling error requests

HttpResponse::Status	HttpResponse::handleErrorRequest(ushort statusCode)
{
	_statusCode = statusCode;

	if (!canServeCustomErrorPage(statusCode))
	{
		_body = createDefaultStatusPage(statusCode);
		_bodyType = HttpResponse::STRING;
		_headers["Content-Length"] = convToStr(_body.size());
		_headers["Content-Type"] = "text/html";
	}
	return (READY_TO_SEND);
}

TStr					HttpResponse::createDefaultStatusPage(ushort statusCode)
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

bool					HttpResponse::canServeCustomErrorPage(ushort statusCode)
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
HttpResponse::Status		HttpResponse::getStatus(void) const { return _status; }
HttpResponse::BodyType		HttpResponse::getBodyType(void) const { return _bodyType; }
int							HttpResponse::getBodyFd(void) const { return _bodyfd; }
const TStr&					HttpResponse::getBodyStr(void) const { return _body; };


// 11 - Set HttpResponse fields from request
HttpResponse::Status	HttpResponse::prepareResponseFromRequestHeadersComplete(const HttpRequest* httpRequest, const LocationConfig* locationConfig)
{
	_httpRequest = httpRequest;
	_locationConfig = locationConfig;

	// Add default headers used for any Http Response
	setDefaultHeaders();

	// Check HTTP method is allowed in this config
	if (!isRequestHttpMethodAllowed())
	{
		_statusCode = 405;
		handleErrorRequest(_statusCode);
		return 	(READY_TO_SEND);
	}

	// Build resolved filepath
	_resolvedPath = buildResolvedPath();
	if (!isResolvedPathAllowed(_resolvedPath))
	{
		_statusCode = 406;
		handleErrorRequest(_statusCode);
		return (READY_TO_SEND);
	}
	normalizeFilepath(_resolvedPath);

	switch (_httpRequest->getMethod())
	{
		case	HttpMethods::GET:
		case	HttpMethods::HEAD:		return (prepareResponseToGetFromHeaders(_resolvedPath));
		case	HttpMethods::DELETE:	return (prepareResponseToDeleteFromHeaders(_resolvedPath));
		case	HttpMethods::POST:		return (prepareResponseToPostFromHeaders(_resolvedPath));
		case	HttpMethods::PUT:		return (prepareResponseToPutFromHeaders(_resolvedPath));
		default	:						return (READY_TO_SEND);
	}
	return (READY_TO_SEND);
}

HttpResponse::Status	HttpResponse::prepareResponseFromRequestBodyComplete(void)
{
	switch(_httpRequest->getMethod())
	{
		case	HttpMethods::POST:	return (prepareResponseToPostFromBody(_resolvedPath));
		case	HttpMethods::PUT:	return (prepareResponseToPostFromBody(_resolvedPath));
		default :					return (READY_TO_SEND);
	}
	return (READY_TO_SEND);
}



HttpResponse::Status	HttpResponse::prepareResponseInvalidHeadersRequest(unsigned short code)
{
	_body = createDefaultStatusPage(code);
	_bodyType = HttpResponse::STRING;
	_headers["Content-Length"] = convToStr(_body.size());
	_headers["Content-Type"] = "text/html";
	return (READY_TO_SEND);
}

HttpResponse::Status	HttpResponse::prepareResponseInvalidBodyRequest(unsigned short code)
{
	return (handleErrorRequest(code));
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

