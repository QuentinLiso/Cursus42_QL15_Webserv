/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   7B_HttpRequestResolution.cpp                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/03 19:31:18 by qliso             #+#    #+#             */
/*   Updated: 2025/06/23 12:45:34 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "7B_HttpRequestResolution.hpp"

// Constructor, destructor
HttpRequestResolution::HttpRequestResolution(const HttpRequest& httpRequest)
		: 	_httpRequest(httpRequest),
			_httpRequestData(httpRequest.getHttpRequestData()),
			_httpResolutionStatusCode(100),
			_resolutionState(RESOLUTION_PROCESSING),
			_locationConfig(NULL),
			_resolvedPathExists(0)
{}

HttpRequestResolution::~HttpRequestResolution(void) {}

// HttpRequest - Validation against config
HttpRequestResolution::ResolutionState	HttpRequestResolution::isRequestHttpMethodAllowed(void)
{
	bool	allowed = _locationConfig->getAllowedMethods().isAllowedMethod(_httpRequestData.getMethod());
	if (!allowed)
		return (error(405, "Http Method not allowed in requested config", RESOLUTION_INVALID));

	return (RESOLUTION_PROCESSING);
}

TStr									HttpRequestResolution::buildResolvedPath(void)
{
	const Root&	root = _locationConfig->getRoot();
	if (root.isSet())
	{
		return (root.getFolderPath() + _httpRequestData.getUriPath());
	}
	else
	{
		const TStr& uri = _httpRequestData.getUriPath();
		const TStr& aliasPath = _locationConfig->getAlias().getFolderPath();
		const TStr& locationPath = _locationConfig->getLocationPath().getPath();
		return (aliasPath + uri.substr(locationPath.size()));
	}
}

HttpRequestResolution::ResolutionState	HttpRequestResolution::isResolvedPathAllowed(void)
{
	bool allowed = isValidFilepath(_resolvedPath) && !containsDoubleDotsAccess(_resolvedPath);
	if (!allowed)
		return (error(405, "HTTP request URI is an invalid filepath", RESOLUTION_INVALID));

	return (RESOLUTION_PROCESSING);
}

TStr									HttpRequestResolution::getParentDirectory(const TStr& path)
{
	if (path.empty() || path.size() == 1)
		return ("/");

	bool trailingSlash = (path[path.size() - 1] == '/');
	size_t	pos = trailingSlash ? 
			path.find_last_of('/', path.size() - 2) :
			path.find_last_of('/');

	return (pos == TStr::npos ? "/" : path.substr(0, pos));
}

// GET + HEAD
HttpRequestResolution::ResolutionState	HttpRequestResolution::isGetHeadRequestValid(void)
{
	_resolvedPathExists = stat(_resolvedPath.c_str(), &_resolvedPathStat);
	
	if (_resolvedPathExists != 0)	return (error(404, "Requested GET path doesn't exist", RESOLUTION_INVALID));
			
	if (S_ISREG(_resolvedPathStat.st_mode))	// GET file
		return (handleGetHeadFile());
	
	else if (S_ISDIR(_resolvedPathStat.st_mode)) // GET directory
		return (handleGetHeadDirectory());
	
	return (error(404, "Requested GET is not a file nor directory", RESOLUTION_INVALID));
}

HttpRequestResolution::ResolutionState	HttpRequestResolution::handleGetHeadFile(void)
{
	bool	isCgiUri = _locationConfig->getCgiExtensions().contains(getFileExtension(_resolvedPath));
	if (isCgiUri) // GET cgi file
	{
		_resolvedPathParentDir = getParentDirectory(_resolvedPath);
		if (access(_resolvedPathParentDir.c_str(), X_OK) != 0)	// File not executable
			return (error(403, "GET request targets cgi URI without parent folder " + _resolvedPathParentDir + " access rights", RESOLUTION_INVALID));
		
		_httpResolutionStatusCode = 200;
		return (RESOLUTION_VALID_GET_HEAD_CGI);
	}
	else	// GET static fle
	{
		if (access(_resolvedPath.c_str(), R_OK) != 0)
			return (error(403, "Request GET path is forbidden", RESOLUTION_INVALID));
		_httpResolutionStatusCode = 200;
		return (RESOLUTION_VALID_GET_HEAD_STATIC);
	}
}

HttpRequestResolution::ResolutionState	HttpRequestResolution::handleGetHeadDirectory(void)
{
	if (_locationConfig->getIndex().isSet()) // Check index files
	{
		const TStrVect& indexFiles = _locationConfig->getIndex().getFileNames();
		for (size_t i = 0; i < indexFiles.size(); i++)
		{
			TStr	indexFilePath = _resolvedPath + '/' + indexFiles[i];
			normalizeFilepath(indexFilePath);
			Console::log(Console::DEBUG, "Trying index file : " + indexFilePath);

			struct stat	indexFileStat;
			int			indexFileExists = stat(indexFilePath.c_str(), &indexFileStat);

			if (indexFileExists != 0)
				continue ;
			
			if (_locationConfig->getCgiExtensions().contains(getFileExtension(indexFilePath)))
			{
				_resolvedPath = indexFilePath;
				_resolvedPathStat = indexFileStat;
				_resolvedPathExists = indexFileExists;
				return (RESOLUTION_VALID_GET_HEAD_CGI);
			}
			else
			{
				if (access(indexFilePath.c_str(), R_OK) == 0)
				{
					_resolvedPath = indexFilePath;
					_resolvedPathStat = indexFileStat;
					_resolvedPathExists = indexFileExists;
					_httpResolutionStatusCode = 200;
					return (RESOLUTION_VALID_GET_HEAD_STATIC);
				}
			}
		}
	}

	if (_locationConfig->getAutoindex().isActive())	// Check autoindex
		return (RESOLUTION_VALID_GET_HEAD_AUTOINDEX);
	
	return (error(404, "Requested GET directory has no valid index nor autoindex", RESOLUTION_INVALID));
	
}

// DELETE

HttpRequestResolution::ResolutionState	HttpRequestResolution::isDeleteRequestValid(void)
{
	_resolvedPathExists = stat(_resolvedPath.c_str(), &_resolvedPathStat);

	if (_resolvedPathExists != 0)
		return (error(404, "DELETE request targets non existing file", RESOLUTION_INVALID));

	TStr	parentDir = getParentDirectory(_resolvedPath);

	if (access(parentDir.c_str(), X_OK | W_OK) != 0)
			return (error(403, "DELETE target's parent dir is neither read nor write accessible", RESOLUTION_INVALID));

	if (S_ISDIR(_resolvedPathStat.st_mode))
	{
		if (isDirectoryEmpty() == RESOLUTION_INVALID)
			return (RESOLUTION_INVALID);
		
		if (rmdir(_resolvedPath.c_str()) != 0)
			return (error(500, "DELETE target directory rmdir syscall failed", RESOLUTION_INVALID));
		_httpResolutionStatusCode = 204;
		return (RESOLUTION_VALID_DELETE);
	}
	else if (S_ISREG(_resolvedPathStat.st_mode))
	{
		if (unlink(_resolvedPath.c_str()) != 0)
			return (error(500, "DELETE target file unlink syscall failed", RESOLUTION_INVALID));
		_httpResolutionStatusCode = 200;
		return (RESOLUTION_VALID_DELETE);
	}
	return (error(404, "Requested DELETE is not a file nor directory", RESOLUTION_INVALID));
}

HttpRequestResolution::ResolutionState	HttpRequestResolution::isDirectoryEmpty(void)
{
	DIR*	directory = opendir(_resolvedPath.c_str());

	if (directory == NULL)
		return (error(500, "opendir failed for DELETE target directory ", RESOLUTION_INVALID));

	struct dirent	*directoryEntry;
	while ((directoryEntry = readdir(directory)) != NULL)
	{
		TStr	name = directoryEntry->d_name;
		if (name != "." && name != "..")
		{
			closedir(directory);
			return (error(409, "DELETE target directory is not empty", RESOLUTION_INVALID));
		}
	}
	closedir(directory);
	return (RESOLUTION_PROCESSING);
}	

// PUT
HttpRequestResolution::ResolutionState	HttpRequestResolution::isPutRequestValid(void)
{
	if (_httpRequestData.getTransferEncoding() != HttpRequestData::TE_CHUNKED && !_httpRequestData.isContentLengthSet())
		return (error(411, "PUT request with no body", RESOLUTION_INVALID));
	
	_resolvedPathExists = stat(_resolvedPath.c_str(), &_resolvedPathStat);
	_resolvedPathParentDir = getParentDirectory(_resolvedPath);
	bool	isCgiUri = _locationConfig->getCgiExtensions().contains(getFileExtension(_resolvedPath));

	if (_resolvedPathExists != 0)	// File or folder does not exist
	{
		if (access(_resolvedPathParentDir.c_str(), W_OK | X_OK) != 0)
				return (error(403, "Forbidden access write/exec to parent dir " + _resolvedPathParentDir + " for PUT request on inexistent file ", RESOLUTION_INVALID));
		_httpResolutionStatusCode = 201;
		return (isCgiUri ? RESOLUTION_VALID_PUT_CGI : RESOLUTION_VALID_PUT_STATIC);
	}
	else
	{
		if (S_ISDIR(_resolvedPathStat.st_mode)) // Existing URI folder
			return (error(405, "PUT request not allowed on existing folder", RESOLUTION_INVALID));
		
		else if (S_ISREG(_resolvedPathStat.st_mode))
		{
			if (isCgiUri)
			{
				if (access(_resolvedPath.c_str(), X_OK) != 0)	// Existing file not accessible
					return (error(403, "PUT request on existing file not write accessible", RESOLUTION_INVALID));
				_httpResolutionStatusCode = 204;
				return (RESOLUTION_VALID_PUT_CGI);
			}
			else
			{
				if (access(_resolvedPath.c_str(), W_OK) != 0)	// Existing file not accessible
					return (error(403, "PUT request on existing file not write accessible", RESOLUTION_INVALID));
				_httpResolutionStatusCode = 204;
				return (RESOLUTION_VALID_PUT_STATIC);
			}
		}
		else
			return (error(404, "PUT request target is neither directory or file", RESOLUTION_INVALID));
	}
}

// POST
HttpRequestResolution::ResolutionState	HttpRequestResolution::isPostRequestValid(void)
{
	_resolvedPathExists = stat(_resolvedPath.c_str(), &_resolvedPathStat);
	_resolvedPathParentDir = getParentDirectory(_resolvedPath);
	bool	isCgiUri = _locationConfig->getCgiExtensions().contains(getFileExtension(_resolvedPath));

	if (_resolvedPathExists != 0)	// Non-existing URI
	{
		if (!isCgiUri)							// Non-existing non-cgi URI
			return (error(404, "POST request targets inexistent non-cgi URI", RESOLUTION_INVALID));
			
		if (access(_resolvedPathParentDir.c_str(), W_OK | X_OK) != 0)	// Parent folder not writeable executable
			return (error(403, "POST request targets inexistent cgi URI without parent folder access rights", RESOLUTION_INVALID));
		
	}
	else	// Existing URI
	{
		if (S_ISDIR(_resolvedPathStat.st_mode)) // Existing URI folder
			return (error(405, "POST request targets a directory", RESOLUTION_INVALID));
		else if (S_ISREG(_resolvedPathStat.st_mode))	// Existing URI file
		{
			if (!isCgiUri)		// Existing non-cgi URI 
				return (error(405, "POST request targets a non-cgi file", RESOLUTION_INVALID));

			if (access(_resolvedPath.c_str(), X_OK) != 0) // File not writeable
				return (error(403, "Access forbidden to POST request target file", RESOLUTION_INVALID));
		} 
		else
			return (error(403, "POST request targets neither directory nor file", RESOLUTION_INVALID));
	}

	if (_httpRequestData.getTransferEncoding() != HttpRequestData::TE_CHUNKED && !_httpRequestData.isContentLengthSet())
		return (error(411, "POST request requires a body", RESOLUTION_INVALID));

	return (RESOLUTION_VALID_POST_CGI);
}


// Error
HttpRequestResolution::ResolutionState	HttpRequestResolution::error(ushort httpResolutionStatusCode, const TStr& step, HttpRequestResolution::ResolutionState resolutionState)
{
	std::ostringstream	oss;
	oss << "Invalid request : " << step << " - Error code : " << httpResolutionStatusCode;
	Console::log(Console::DEBUG, oss.str());
	
	_resolutionState = resolutionState;
	_httpResolutionStatusCode = httpResolutionStatusCode;
	return (_resolutionState);
}


// Public
HttpRequestResolution::ResolutionState	HttpRequestResolution::resolveHttpRequest(const ListeningSocket* const listeningSocket)
{
	if (_httpRequest.getHttpRequestState() == HttpRequest::PARSING_HEADERS_INVALID || _httpRequestData.getHostAddress().empty())
		return (error(_httpRequestData.getHttpRequestDataStatusCode(), "Headers wrong", RESOLUTION_INVALID));

	_locationConfig = listeningSocket->findLocationConfig(_httpRequestData.getHostAddress(), _httpRequestData.getUriPath());
	
	if (isRequestHttpMethodAllowed() == RESOLUTION_INVALID) return (RESOLUTION_INVALID);

	_resolvedPath = buildResolvedPath();
	if (isResolvedPathAllowed() == RESOLUTION_INVALID) return (RESOLUTION_INVALID);
	normalizeFilepath(_resolvedPath);

	switch (_httpRequestData.getMethod())
	{
		case HttpMethods::GET:		
		case HttpMethods::HEAD:		_resolutionState = isGetHeadRequestValid();	return (_resolutionState);
		case HttpMethods::POST:		_resolutionState = isPostRequestValid();	return (_resolutionState);
		case HttpMethods::PUT:		_resolutionState = isPutRequestValid();		return (_resolutionState);
		case HttpMethods::DELETE:	_resolutionState = isDeleteRequestValid();	return (_resolutionState);
		default :					return (error(405, "Method not allowed", RESOLUTION_INVALID));
	}
	
	return (RESOLUTION_INVALID);
}

// Getters
unsigned short				HttpRequestResolution::getHttpResolutionStatusCode(void) const { return _httpResolutionStatusCode; }		
HttpRequestResolution::ResolutionState				HttpRequestResolution::getResolutionState(void) const { return _resolutionState; }
const LocationConfig*		HttpRequestResolution::getLocationConfig(void) const { return _locationConfig; }
const TStr&					HttpRequestResolution::getResolvedPath(void) const { return _resolvedPath; }
const struct stat&			HttpRequestResolution::getResolvedPathStat(void) const { return _resolvedPathStat; }
const TStr&					HttpRequestResolution::getParentDirectory(void) const { return _resolvedPathParentDir; }

// Print
// std::ostream&	HttpRequest::printRequest(std::ostream& o) const
// {
// 	o	<< "Method : " << _method << '\n'
// 		<< "URI Path : " << _uriPath << '\n'
// 		<< "URI Query : " << _uriQuery << '\n'
// 		<< "Version : " << _version << '\n'
// 		<< "Host Address : " << _hostAddress << '\n'
// 		<< "Host Port : " << _hostPort << '\n'
// 		<< "Content Length : " << _contentLength << '\n'
// 		<< "User Agent : " << _userAgent << '\n'
// 		<< "Connection : " << _connection << '\n'
// 		<< "Content Type : " << _contentType << '\n'
// 		<< "multipart/form-data boundary : " << _multipartBoundary << '\n'
// 		<< "Cookies :";

// 	for (std::map<TStr, TStr>::const_iterator it = _cookies.begin(); it != _cookies.end(); it++)
// 		o << " " << it->first << "=" << it->second;
// 	o 	<< '\n'
// 		<< "Referer : " << _referer << '\n'
// 		<< "Content Encoding : " << _contentEncoding << '\n'
// 		<< "Transfer Encoding : " << _transferEncoding << '\n'
// 		<< "Body : " << _body << std::endl;
// 	return (o);
// }



