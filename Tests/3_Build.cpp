/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_Build.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/22 08:56:11 by qliso             #+#    #+#             */
/*   Updated: 2025/05/28 23:15:27 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "3_Build.hpp"

//==================== ELEMENTS

ElementConfig::ElementConfig(void) : _line(0), _column(0), _name(""), _args(), _error(0) {}

ElementConfig::ElementConfig(Statement* statement)
		: 	_line(statement->getLine()),
			_column(statement->getColumn()), 
			_name(statement->getName()), 
			_args(statement->getArgs()), 
			_error(0)
{}

ElementConfig::ElementConfig(const ElementConfig& c) : _line(c._line), _column(c._column), _name(c._name), _args(c._args), _error(c._error) {}

ElementConfig& ElementConfig::operator=(const ElementConfig& c)
{
	if (this != &c)
	{
		_line = c._line;
		_column = c._column;
		_name = c._name;
		_args = c._args;
		_error = c._error;
	}
	return (*this);
}

ElementConfig::~ElementConfig(void) {}

int				ElementConfig::getLine(void) const { return _line; }
int				ElementConfig::getColumn(void) const { return _column; }
const TStr&		ElementConfig::getName(void) const { return _name; }
const TStrVect&	ElementConfig::getArgs(void) const { return _args; }
int				ElementConfig::getError(void) const { return _error; }
void			ElementConfig::setError(int error) { _error = error; }

void			ElementConfig::updateElementConfig(Statement* statement)
{
	_line = statement->getLine();
	_column = statement->getColumn();
	_name = statement->getName();
	_args = statement->getArgs();
}

int				ElementConfig::error(const TStr& msg)
{
	Console::configLog(Console::ERROR, _line, _column, _name, _args, "BUILDING", msg);
	setError(1);
	return (1);
}

void			ElementConfig::warning(const TStr& msg)
{
	Console::configLog(Console::WARNING, _line, _column, _name, _args, "BUILDING", msg);
}




// =========================== DIRECTIVES

// ===== LISTEN 

int	Listen::setHostAndPort(const TStrVect& args)
{
	if (args.size() != 1)
		return(error("Invalid number of arguments for listen directive"));
	
	std::size_t	sep = args[0].find(':');
	if (sep == std::string::npos)
	{
		warning("Server listen directive has no IP address defined, adding '0.0.0.0' by default");
		_host = "0.0.0.0";
		return (setPort(args[0]));
	}
	else
	{
		setIp(args[0].substr(0, sep));
		return (setPort(args[0].substr(sep + 1)));
	}
}

int	Listen::setPort(const std::string& arg)
{
	unsigned short	portValue;
	bool			isValid = strToVal<unsigned short>(arg, portValue);
	
	if (!isValid || portValue == 0)
	return (error("Invalid port, port must be a value between 1 and 65535"));
	_port = portValue;
	return (0);
}

int	Listen::setIp(const std::string& ip)
{
	if (ip == "localhost")
	{
		_host = "127.0.0.1";
		return (0);
	}
	
	TStrVect	fields = split(ip, ".");
	if (fields.size() != 4)
	return (error("Invalid IP address"));
	for (size_t	i = 0; i < 4; i++)
	{
		unsigned char	val;
		if (strToVal<u_char>(fields[i], val) == false)
		return (error("Invalid IP address"));
	}
	_host = ip;
	return (0);
}

Listen::Listen(void) :  ElementConfig(), _host(""), _port(0), _set(false) {}

Listen::Listen(const Listen& c) : ElementConfig(c), _host(c._host), _port(c._port), _set(c._set) {}

Listen& Listen::operator=(const Listen& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_host = c._host;
		_port = c._port;
		_set = c._set;
	}
	return (*this);
}

Listen::~Listen(void) {}

const TStr&	Listen::getHost(void) const { return _host; }
ushort	Listen::getPort(void) const { return _port; }
bool	Listen::isSet(void) const { return _set; }

int	Listen::set(Statement* statement)
{
	updateElementConfig(statement);
	if (_set == true)
		return (error("Duplicate listen statement"));
	_set = true;
	return (setHostAndPort(statement->getArgs()));
}



std::ostream&	Listen::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Host : " << _host << " Port : " << _port << std::endl;
	return (o);
}

// ===== SERVER NAMES

int	ServerName::addServerNames(const TStrVect& args)
{
	if (args.size() == 0)
		return (error("Missing arguments in server_name directive"));
	for (size_t i = 0; i < args.size(); i++)
	{
		const TStr& arg = args[i];
		if (!arg.empty())
		{
			if (_serverNames.insert(arg).second == false)
				error("Duplicate server_name argument '" + arg + "' found in same block");
		}
	}
	return (0);
}

ServerName::ServerName(void) : ElementConfig(), _serverNames(), _set(false) {}

ServerName::ServerName(const ServerName& c) : ElementConfig(c), _serverNames(c._serverNames), _set(c._set) {}

ServerName& ServerName::operator=(const ServerName& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_serverNames = c._serverNames;
		_set = c._set;
	}
	return (*this);
}

ServerName::~ServerName(void) {}

const std::set<TStr>& ServerName::getServerNames(void) const { return _serverNames; }
bool	ServerName::hasServerName(const TStr& name) const { return _serverNames.find(name) != _serverNames.end(); }
bool	ServerName::isSet(void) const { return _set; }

int		ServerName::set(Statement* statement)
{
	updateElementConfig(statement);
	if (_set == true)
		warning("Duplicate server_name directive found in same block");
	_set == true;
	return (addServerNames(statement->getArgs()));
}

std::ostream&	ServerName::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Server names:";
	for (std::set<TStr>::const_iterator it = _serverNames.begin() ; it != _serverNames.end(); it++)
		o << " " << *it;
	o << std::endl;

	return (o);
}

// === LOCATION PATH

int	LocationPath::setPath(const TStrVect& args)
{
	if (args.size() != 1 || args[0].empty())
		return (error("Missing or invalid path argument in location statement"));
		
	_path = args[0];
	int	errorFound = 0;
	
	if (_path[0] != '/')
		errorFound = error("Location path argument must start with a '/'");
	if (!isValidFilepath(_path))
		errorFound = error("Invalid character found in location path");
	if (containsDoubleDotsAccess(_path))
		errorFound = error("Location path cannot have '..' as path access inside the provided argument");

	removeStrDuplicateChar(_path, '/');
	removeDotPaths(_path);
	return (errorFound);
}

LocationPath::LocationPath(void) : ElementConfig(), _path(""), _set(false) {}

LocationPath::LocationPath(const LocationPath& c) : ElementConfig(c), _path(c._path), _set(c._set) {}

LocationPath& LocationPath::operator=(const LocationPath& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_path = c._path;
		_set = c._set;
	}
	return (*this);
}

LocationPath::~LocationPath(void) {}

const TStr& LocationPath::getPath(void) const { return _path; };
bool		LocationPath::isSet(void) const { return _set; };

int	LocationPath::set(Statement* statement)
{
	updateElementConfig(statement);
	_set = true;
	return (setPath(statement->getArgs()));
}

std::ostream&	LocationPath::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Path : " << _path << std::endl;
	return (o);
}


// === ALIAS

int	Alias::setFolderPath(const TStrVect& args)
{
	if (args.size() != 1 || args[0].empty())
		return (error("Alias directive must have exactly 1 non-empty argument"));
	
	_folderPath = args[0];
	int errorFound = 0;
	
	if (_folderPath[0] != '/' && _folderPath[_folderPath.size() - 1] != '/')
		errorFound = error("Alias directive must start and end with a '/'");
	if (!isValidFilepath(_folderPath))
		errorFound = error("Invalid folder path character found in alias directive");
	if (containsDoubleDotsAccess(_folderPath))
		errorFound = error("Alias directive cannot have '..' as path access inside the provided argument");
	// if (!isExecutableDirectory(folderPath))
	// 	errorFound = error("Alias directive folder path is not an accessible directory");

	removeStrDuplicateChar(_folderPath, '/');
	removeDotPaths(_folderPath);
	return (errorFound);
}

Alias::Alias(void) : ElementConfig(), _folderPath(""), _set(false) {}

Alias::Alias(const Alias& c) : ElementConfig(c), _folderPath(c._folderPath), _set(c._set) {}

Alias& Alias::operator=(const Alias& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_folderPath = c._folderPath;
		_set = c._set;
	}
	return (*this);
}

Alias::~Alias(void) {}

const TStr& Alias::getFolderPath(void) const { return _folderPath; }
bool	Alias::isSet(void) const { return _set; }

int	Alias::set(Statement* statement)
{
	updateElementConfig(statement);
	if (_set)
		return (error("Duplicate alias directive found in the same block"));
	_set = true;
	return (setFolderPath(statement->getArgs()));
}

std::ostream&	Alias::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Alias : " << _folderPath << std::endl;
	return (o);
}

// === ALLOWED METHODS

int	AllowedMethods::addAllowedMethods(const TStrVect& args)
{
	if (args.size() == 0)
		return (error("Missing arguments in allowed_methods directive"));
	
	for (size_t i = 0; i < args.size(); i++)
	{
		const TStr& arg = args[i];
		HttpMethods::Type	method;
		if (HttpMethodsMap::map().find(arg, method) == false)
		{
			error("Invalid HTTP method '" + arg + "' found in allowed_methods directive");
			continue ;
		}
		if (_allowedMethods.insert(method).second == false)
		{
			warning("Duplicate HTTP method '" + arg + "' found in allowed_methods directive");
			continue ;
		}
		_allowedMethodsStr.insert(arg);
	}
	return (0);
}

AllowedMethods::AllowedMethods(void) : ElementConfig(), _allowedMethods(), _set(false) {}

AllowedMethods::AllowedMethods(const AllowedMethods& c) : ElementConfig(c), _allowedMethods(c._allowedMethods), _set(c._set) {}

AllowedMethods& AllowedMethods::operator=(const AllowedMethods& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_allowedMethods = c._allowedMethods;
		_allowedMethodsStr = c._allowedMethodsStr;
		_set = c._set;
	}
	return (*this);
}

AllowedMethods::~AllowedMethods(void) {}

const std::set<HttpMethods::Type>& AllowedMethods::getAllowedMethods(void) const { return _allowedMethods; }
bool	AllowedMethods::isAllowedMethod(HttpMethods::Type method) const { return _allowedMethods.find(method) != _allowedMethods.end(); }
bool	AllowedMethods::isAllowedMethod(const TStr& method) const { return _allowedMethodsStr.find(method) != _allowedMethodsStr.end(); }
bool	AllowedMethods::isSet(void) const { return _set; }

void	AllowedMethods::setDefaultMethods(void)
{
	_allowedMethods.insert(HttpMethods::GET);
	_allowedMethods.insert(HttpMethods::POST);
	_allowedMethodsStr.insert("GET");
	_allowedMethodsStr.insert("POST");
	_set = true;
}

int	AllowedMethods::set(Statement* statement)
{
	updateElementConfig(statement);
	if (_set)
		warning("Duplicate allowed_methods directive found in same block");
	_set = true;
	return (addAllowedMethods(statement->getArgs()));
}

std::ostream&	AllowedMethods::print(std::ostream& o, size_t indent) const
{
	std::set<HttpMethods::Type>::const_iterator	it = _allowedMethods.begin();
	std::set<HttpMethods::Type>::const_iterator	ite = _allowedMethods.end();

	o << TStr(indent, '-') << "Allowed HTTP methods:";
	for (; it != ite; it++)
	{
		TStr method;
		HttpMethodsMap::map().find(*it, method);
		o << " " << method;
	}
	o << std::endl;

	return (o);
}

// === CGI PASS

int		CgiPass::setFolderPath(const TStrVect& args)
{
	if (args.size() != 1 || args[0].empty())
		return (error("cgi_pass directive should have exactly 1 non-empty argument"));
	
	int	errorFound = 0;
	_filePath = args[0];
	if (_filePath[0] != '/')
		errorFound = error("cgi_pass directive argument must be an absolute path and must start with '/'");
	if (!isValidFilepath(_filePath))
		errorFound = error("Invalid folder path character found in cgi_pass directive");
	if (containsDoubleDotsAccess(_filePath))
		errorFound = error("cgi_pass directive cannot have '..' as path access inside the provided argument");
	// if (checkCgiBin(filePath))
	// 	errorFound = 1;
	// if (checkCgiDirectory(filePath))
	// 	errorFound = 1;
	removeStrDuplicateChar(_filePath, '/');
	removeDotPaths(_filePath);
	return (errorFound);
}

int	CgiPass::checkCgiBin(const TStr& filePath)
{
	struct stat	fileStatus;
	
	if (stat(filePath.c_str(), &fileStatus) != 0)
		return (error("cgi_pass file does not exist"));
	
	if (!S_ISREG(fileStatus.st_mode))
		return (error("cgi_pass file is not a file"));
	
	if (access(filePath.c_str(), X_OK))
		return (error("cgi_pass file is not executable"));
	return (0);
}

int	CgiPass::checkCgiDirectory(const TStr& filePath)
{
	struct stat dirStatus;
	
	TStr dir = filePath.substr(0, filePath.find_last_of('/'));	
	if (stat(dir.c_str(), &dirStatus) == 0 && dirStatus.st_mode & S_IWOTH)
		return (error("cgi_pass file directory is world writable, unsafe for cgi_pass"));
	return (0);
}

CgiPass::CgiPass(void) : ElementConfig(), _filePath(""), _fullFilePath(""), _set(false) {}

CgiPass::CgiPass(const CgiPass& c) : ElementConfig(c), _filePath(c._filePath), _fullFilePath(c._fullFilePath), _set(c._set) {}

CgiPass& CgiPass::operator=(const CgiPass& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_filePath = c._filePath;
		_set = c._set;
	}
	return (*this);
}

CgiPass::~CgiPass(void) {}

const TStr& CgiPass::getFilePath(void) const { return _filePath; }
const TStr& CgiPass::getFullFilePath(void) const { return _fullFilePath; }
bool		CgiPass::isSet(void) const { return _set; }

int	CgiPass::set(Statement* statement)
{
	updateElementConfig(statement);
	if (_set)
		return (error("Duplicate cgi_pass directive found in same block"));
	_set = true;
	return (setFolderPath(statement->getArgs()));
}

int CgiPass::setFullPath(const TStr& configFullPath)
{
	_fullFilePath = joinPaths(configFullPath, _filePath);

	int	errorFound = checkCgiBin(_fullFilePath);
	errorFound = checkCgiDirectory(_fullFilePath);
	return (errorFound);
}

std::ostream&	CgiPass::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Cgi Pass: " << _filePath << "\t-> Full path : " << _fullFilePath << std::endl;
	return (o);
}


// ==== ROOT

int		Root::setFolderPath(const TStrVect& args)
{
	if (args.size() != 1 || args[0].empty())
		return (error("root directive must have exactly 1 non-empty argument"));
	
	int	errorFound = 0;
	_folderPath = args[0];
	
	if (_folderPath[0] != '/')
		errorFound = error("root directive argument must start with '/'");
	if (!isValidFilepath(_folderPath))
		errorFound = error("Invalid folder path character found in root directive");
	if (containsDoubleDotsAccess(_folderPath))
		errorFound = error("root directive cannot have '..' as root access inside the provided argument");
	// if (!isExecutableDirectory(folderPath))
	// 	errorFound = error("root directive folder path is not an accessible directory");
	
	normalizeFilepath(_folderPath);
	return (errorFound);
}

Root::Root(void) : ElementConfig(), _folderPath(""), _set(false) {}

Root::Root(const Root& c) : ElementConfig(c), _folderPath(c._folderPath), _set(c._set) {}

Root& Root::operator=(const Root& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_folderPath = c._folderPath;
		_set = c._set;
	}
	return (*this);
}

Root::~Root(void) {}

const TStr& Root::getFolderPath(void) const { return _folderPath; }
bool		Root::isSet(void) const { return _set; }

int		Root::set(Statement* statement)
{
	updateElementConfig(statement);
	if (_set)
		return (error("Duplicate root directive found in the same block"));
	_set = true;
	return (setFolderPath(statement->getArgs()));
}

std::ostream&	Root::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Root: " << _folderPath << std::endl;
	return (o);
}


// === INDEX

int		Index::setFileNames(const TStrVect& args)
{
	if (args.size() == 0)
		return (error("Index directive must have at lease 1 argument"));
	
	for (size_t i = 0; i < args.size(); i++)
	{
		const TStr& fileName = args[i];
		if (fileName.empty() || fileName.find('/') != std::string::npos || fileName == "." || fileName == "..")
		{
			warning("Invalid filename '" + fileName + "' in index directive");
			continue ;
		}
		_fileNames.push_back(fileName);
	}
	if (_fileNames.empty())
		warning("No correct filename found in the index directive, index is empty then");
	return (0);
}

Index::Index(void) : ElementConfig(), _fileNames(), _set(false) {}

Index::Index(const Index& c) : ElementConfig(c), _fileNames(c._fileNames), _set(c._set) {}

Index& Index::operator=(const Index& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_fileNames = c._fileNames;
		_set = c._set;
	}
	return (*this);
}

Index::~Index(void) {}

const	TStrVect& Index::getFileNames(void) const { return _fileNames; }
bool	Index::isSet(void) const { return _set; }

int		Index::set(Statement* statement)
{
	updateElementConfig(statement);
	if (_set)
		return (error("Duplicate index directive found in the same block"));
	_set = true;
	return (setFileNames(statement->getArgs()));
}

std::ostream&	Index::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Index :";
	for (size_t i = 0; i < _fileNames.size(); i++)
		o << " " << _fileNames[i];
	o << std::endl;
	return (o);
}


// === AUTOINDEX

int Autoindex::setAutoIndex(const TStrVect& args)
{
	if (args.size() != 1)
		return (error("Autoindex directive must have exactly 1 argument 'on' or 'off'"));
	
	const TStr& arg = args[0];
	if (arg == "on")
	{
		_active = true;
		return (0);
	}
	else if (arg == "off")
	{
		_active = false;
		return (0);
	}
	else
		return (error("Autoindex directive must have exactly 1 argument 'on' or 'off'"));
}

Autoindex::Autoindex(void) : ElementConfig(), _active(false), _set(false) {}

Autoindex::Autoindex(const Autoindex& c) : ElementConfig(c), _active(c._active), _set(c._set) {}

Autoindex& Autoindex::operator=(const Autoindex& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_active = c._active;
		_set = c._set;
	}
	return (*this);
}

Autoindex::~Autoindex(void) {}

bool	Autoindex::isActive(void) const { return _active; }
bool	Autoindex::isSet(void) const { return _set; };

int Autoindex::set(Statement* statement)
{
	updateElementConfig(statement);
	if (_set)
		return (error("Duplicate autoindex directive found in the same block"));
	_set = true;
	return (setAutoIndex(statement->getArgs()));
}

std::ostream&	Autoindex::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Autoindex : " << (_active ? "on" : "off") << std::endl;
	return (o);
}

// === ERROR PAGE

int	ErrorPage::checkValidUri(const TStr& uri)
{
	if (uri.empty())
		return (error("URI cannot be empty in error_pages directive"));
	if (uri[0] == '/' && uri.size() > 1)
	{
		if (containsDoubleDotsAccess(uri))
			return (error("URI '" + uri + "' cannot contain double dots access in error_pages directive"));
		// if (!isExistingAndAccessibleFile(uri, R_OK))
		// 	return (error("URI '" + uri + "' is not an existing read-only accessible file"));
		return (0);
	}
	return(error("Invalid URI '" + uri + "' in error_pages directive"));
}

int	ErrorPage::addErrorPages(const TStrVect& args)
{
	if (args.size() < 2)
		return (error("error_pages directive must have at least 2 arguments"));
	
	size_t	last = args.size() - 1;
	checkValidUri(args[last]);
	int	errorFound = 0;
	
	for (size_t i = 0; i < last; i++)
	{
		const TStr&	arg = args[i];
		ushort	httpErrnum;
		if (strToVal(arg, httpErrnum) == false || httpErrnum < 400 || httpErrnum > 599)
		{
			errorFound = error("Invalid http error code '" + arg + "' in error_pages directive, http error code must be a value between 400 and 599");
			continue ;
		}		
		if (_errorPages.find(httpErrnum) != _errorPages.end())
			warning("Error code '" + arg + "' already mapped earlier in error_pages directive, overwritten with current directive");
		_errorPages[httpErrnum] = args[last];
	}
	return (errorFound);
}

ErrorPage::ErrorPage(void) : ElementConfig(), _errorPages() {}

ErrorPage::ErrorPage(const ErrorPage& c) : ElementConfig(c), _errorPages(c._errorPages) {}

ErrorPage& ErrorPage::operator=(const ErrorPage& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_errorPages = c._errorPages;
	}
	return (*this);
}

ErrorPage::~ErrorPage(void) {}

const std::map<ushort, TStr>& ErrorPage::getErrorPages(void) const { return _errorPages; }

const TStr& ErrorPage::getErrorPage(ushort errnum) const
{ 
	return (_errorPages.find(errnum)->second);
}

bool	ErrorPage::isSet(void) const { return !_errorPages.empty(); }

void		ErrorPage::inheritErrorPages(const ErrorPage& c)
{
	if (!c.isSet())
		return ;

	std::map<ushort, TStr>::const_iterator it = c._errorPages.begin();
	std::map<ushort, TStr>::const_iterator ite = c._errorPages.end();
	
	for (; it != ite; it++)
	{
		std::map<ushort, TStr>::const_iterator	key = _errorPages.find(it->first);
		if (key == _errorPages.end())
			_errorPages[key->first] = key->second;
	}
}

int		ErrorPage::set(Statement* statement)
{
	updateElementConfig(statement);
	return (addErrorPages(statement->getArgs()));
}

std::ostream&	ErrorPage::print(std::ostream& o, size_t indent) const
{
	std::map<ushort, TStr>::const_iterator it = _errorPages.begin();
	std::map<ushort, TStr>::const_iterator ite = _errorPages.end();

	o << TStr(indent, '-') << "Error pages:\n";
	for (; it != ite; it++)
		o << TStr(indent + 2, '-') << it->first << ": " << it->second << std::endl;
	
	return (o);
}

// === CLIENT MAX BODY SIZE

int	ClientMaxBodySize::setBytes(const TStrVect& args)
{
	if (args.size() != 1 || args[0].empty())
		return (error("client_max_body_size directive must have exactly 1 non-empty argument"));
	
	const TStr& arg = args[0];
	if (strToBytes(arg, _maxBytes) == false || _maxBytes == 0)
		return (error("Invalid bytes number '" + arg + "' in client_max_body_size directive, bytes must be a number between 1 and 100M"));
	return (0);
}

ClientMaxBodySize::ClientMaxBodySize(void) : ElementConfig(), _maxBytes(8192), _set(false) {}

ClientMaxBodySize::ClientMaxBodySize(const ClientMaxBodySize& c) : ElementConfig(c), _maxBytes(c._maxBytes), _set(c._set) {}

ClientMaxBodySize& ClientMaxBodySize::operator=(const ClientMaxBodySize& c)
{
	if (this != &c)
	{
		ElementConfig::operator=(c);
		_maxBytes = c._maxBytes;
		_set = c._set;
	}
	return (*this);
}

ClientMaxBodySize::~ClientMaxBodySize(void) {}

size_t	ClientMaxBodySize::getMaxBytes(void) const { return _maxBytes; }
bool	ClientMaxBodySize::isSet(void) const { return _set; }

int	ClientMaxBodySize::set(Statement* statement)
{
	updateElementConfig(statement);
	if (_set)
		return (error("Duplicate client max body size directive found in the same block"));
	_set = true;
	return (setBytes(statement->getArgs()));
}

std::ostream&	ClientMaxBodySize::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Client max body size bytes: " << _maxBytes << std::endl;
	return (o);
}



// ======================== LOCATION CONFIG

LocationConfig::LocationConfig(Statement* statement)
		: 	ElementConfig(statement), 
			_locationPath(), 
			_alias(), 
			_allowedMethods(), 
			_cgiPass(), 
			_root(), 
			_index(), 
			_autoindex(), 
			_errorPage(), 
			_clientMaxBodySize()
{
	setError(setLocationPath(statement));
}

LocationConfig::~LocationConfig(void) {}

const LocationPath&			LocationConfig::getLocationPath(void) const { return _locationPath; }
const TStr&					LocationConfig::getFullPath(void) const { return _fullPath; }
const Alias&				LocationConfig::getAlias(void) const { return _alias; }
const AllowedMethods&		LocationConfig::getAllowedMethods(void) const { return _allowedMethods; }
const CgiPass&				LocationConfig::getCgiPass(void) const { return _cgiPass; }
const Root&					LocationConfig::getRoot(void) const { return _root; }
const Index&				LocationConfig::getIndex(void) const { return _index; }
const Autoindex&			LocationConfig::getAutoindex(void) const { return _autoindex; }
const ErrorPage&			LocationConfig::getErrorPage(void) const { return _errorPage; }
const ClientMaxBodySize&	LocationConfig::getClientMaxBodySize(void) const { return _clientMaxBodySize; }

int	LocationConfig::setLocationPath(Statement* statement) { return(_locationPath.set(statement)); }
void LocationConfig::setFullPath(void) { _fullPath = (_alias.isSet() ? _alias.getFolderPath() : joinPaths(_root.getFolderPath(), _locationPath.getPath())); }
int	LocationConfig::setAlias(Statement* statement) { return (_alias.set(statement)); }
int	LocationConfig::setAllowedMethods(Statement* statement) { return (_allowedMethods.set(statement)); }
int	LocationConfig::setCgiPass(Statement* statement) { return (_cgiPass.set(statement)); }
int	LocationConfig::setRoot(Statement* statement) { return (_root.set(statement)); }
int	LocationConfig::setIndex(Statement* statement) { return (_index.set(statement)); }
int	LocationConfig::setAutoindex(Statement* statement) { return (_autoindex.set(statement)); }
int	LocationConfig::setErrorPage(Statement* statement) { return (_errorPage.set(statement)); }
int	LocationConfig::setClientMaxBodySize(Statement* statement) { return (_clientMaxBodySize.set(statement)); }

int	LocationConfig::setDirective(Statement* statement)
{
	static std::map<TStr, int (LocationConfig::*)(Statement*)>	setDirectiveMap;
	if (setDirectiveMap.empty())
	{
		setDirectiveMap["alias"] = &LocationConfig::setAlias;
		setDirectiveMap["allowed_methods"] = &LocationConfig::setAllowedMethods;
		setDirectiveMap["cgi_pass"] = &LocationConfig::setCgiPass;
		setDirectiveMap["root"] = &LocationConfig::setRoot;
		setDirectiveMap["index"] = &LocationConfig::setIndex;
		setDirectiveMap["autoindex"] = &LocationConfig::setAutoindex;
		setDirectiveMap["error_page"] = &LocationConfig::setErrorPage;
		setDirectiveMap["client_max_body_size"] = &LocationConfig::setClientMaxBodySize;
	}

	std::map<TStr, int (LocationConfig::*)(Statement*)>::const_iterator	it = setDirectiveMap.find(statement->getName());
	if (it == setDirectiveMap.end())
	{
		setError(statement->error("Invalid directive in location block"));
		return (_error);
	}
	return ( (this->*(it->second))(statement) );
}

int	LocationConfig::inheritFromServerConfig(ServerConfig* serverConfig)
{
	if (!_alias.isSet() && !_root.isSet())
		_root = serverConfig->getRoot();
	if (!_index.isSet())
		_index = serverConfig->getIndex();
	if (!_autoindex.isSet())
		_autoindex = serverConfig->getAutoindex();
	_errorPage.inheritErrorPages(serverConfig->getErrorPage());
	if (!_clientMaxBodySize.isSet())
		_clientMaxBodySize = serverConfig->getClientMaxBodySize();
	return (0);
}

int	LocationConfig::validLocationConfig(void)
{
	if (!_alias.isSet() && !_root.isSet())
		return (error("Empty root at server level is not covered by root or alias at location level"));
	
	setFullPath();
	
	if (!isExecutableDirectory(_fullPath))
		return (error("Location full path '" + _fullPath + "' is not an executable directory"));
	
	if (!_allowedMethods.isSet())
		_allowedMethods.setDefaultMethods();
	return (0);
}

std::ostream&	LocationConfig::print(std::ostream& o, size_t indent) const
{
	o << TStr(indent, '-') << "Location: " << std::endl;
	_locationPath.print(o, indent + 3);
	o << TStr(indent + 3, '-') << "Full path : " << _fullPath << std::endl;
	_alias.print(o, indent + 3);
	_allowedMethods.print(o, indent + 3);
	_cgiPass.print(o, indent + 3);
	_root.print(o, indent + 3);
	_index.print(o, indent + 3);
	_autoindex.print(o, indent + 3);
	_errorPage.print(o, indent + 3);
	_clientMaxBodySize.print(o, indent + 3);
	return (o);
}

// ====================== SERVER CONFIG =======================

ServerConfig::ServerConfig(Statement* statement) 
	: 	ElementConfig(statement), 
		_listen(),
		_serverName(),
		_root(),
		_index(),
		_autoindex(),
		_errorPage(),
		_locations()
{
	if (!_args.empty())
		error("Server block cannot have arguments");
}

ServerConfig::~ServerConfig(void)
{
	for (size_t i = 0; i < _locations.size(); i++)
		delete _locations[i];
	_locations.clear();
}

const Listen&			ServerConfig::getListen		(void) const { return _listen; }
const ServerName&		ServerConfig::getServerName	(void) const { return _serverName; }
const Root&				ServerConfig::getRoot			(void) const { return _root; }
const Index&			ServerConfig::getIndex		(void) const { return _index; }
const Autoindex& 		ServerConfig::getAutoindex	(void) const { return _autoindex; }
const ErrorPage&		ServerConfig::getErrorPage	(void) const { return _errorPage; }
const ClientMaxBodySize&	ServerConfig::getClientMaxBodySize(void) const { return _clientMaxBodySize; }
const std::vector<LocationConfig*>	ServerConfig::getLocations(void) const { return _locations; }

int	ServerConfig::setListen(Statement* statement) { return (_listen.set(statement)); }
int	ServerConfig::setServerName(Statement* statement) { return (_serverName.set(statement)); }
int	ServerConfig::setRoot(Statement* statement) { return (_root.set(statement)); }
int	ServerConfig::setIndex(Statement* statement) { return (_index.set(statement)); }
int	ServerConfig::setAutoindex(Statement* statement) { return (_autoindex.set(statement)); }
int	ServerConfig::setErrorPage(Statement* statement) { return (_errorPage.set(statement)); }
int	ServerConfig::setClientMaxBodySize(Statement* statement) { return (_clientMaxBodySize.set(statement)); }

int	ServerConfig::setDirective(Statement* statement)
{
	static std::map<TStr, int (ServerConfig::*)(Statement*)>	setDirectiveMap;
	if (setDirectiveMap.empty())
	{
		setDirectiveMap["listen"] = &ServerConfig::setListen;
		setDirectiveMap["server_name"] = &ServerConfig::setServerName;
		setDirectiveMap["root"] = &ServerConfig::setRoot;
		setDirectiveMap["index"] = &ServerConfig::setIndex;
		setDirectiveMap["autoindex"] = &ServerConfig::setAutoindex;
		setDirectiveMap["error_page"] = &ServerConfig::setErrorPage;
		setDirectiveMap["client_max_body_size"] = &ServerConfig::setClientMaxBodySize;
	}

	std::map<TStr, int (ServerConfig::*)(Statement*)>::const_iterator	it = setDirectiveMap.find(statement->getName());
	if (it == setDirectiveMap.end())
	{
		setError(statement->error("Invalid directive in server block"));
		return (_error);
	}	
	return ( (this->*(it->second))(statement) );
}

void	ServerConfig::addLocation(LocationConfig* locationConfig)
{
	_locations.push_back(locationConfig);
}

void	ServerConfig::makeLocationsInhertiance(void)
{
	for (size_t i = 0; i < _locations.size(); i++)
	{
		_locations[i]->inheritFromServerConfig(this);
		_locations[i]->validLocationConfig();
	}
}

std::ostream&		ServerConfig::print(std::ostream& o, size_t indent = 0) const
{
	o << "Server: " << std::endl;
	_listen.print(o, indent + 3);
	_serverName.print(o, indent + 3);
	_root.print(o, indent + 3);
	_index.print(o, indent + 3);
	_autoindex.print(o, indent + 3);
	_errorPage.print(o, indent + 3);
	_clientMaxBodySize.print(o, indent + 3);
	for (size_t i = 0; i < _locations.size(); i++)
		_locations[i]->print(o, indent + 3);
	return (o);
}


// ======================= BUILDER =============================

void	Builder::parsingToBuild(void)
{
	for (size_t i = 0; i < _statements.size(); i++)
	{
		Statement* statement = _statements[i];
		if (statement->getStatementType() == Statement::DIRECTIVE)
		{
			error(statement, "Expected block node at top level");
			continue ;
		}
		if (statement->getName() != "server")
		{
			error(statement, "Expected server block at top level");
			continue ;
		}
		ServerConfig*	serverConfig = buildServerConfig(statement);
		_build.push_back(serverConfig);
		setError(serverConfig->getError());
	}
}

ServerConfig* 	Builder::buildServerConfig(Statement* statement)
{
	ServerConfig* serverConfig = new ServerConfig(statement);

	const std::vector<Statement *>& serverChildren = statement->getChildren();
	for (size_t i = 0; i < serverChildren.size(); i++)
	{
		Statement* child = serverChildren[i];
		if (child->getStatementType() == Statement::BLOCK)
		{
			if (child->getName() != "location")
			{
				error(child, "Expected location block inside server block");
				continue ;
			}
			LocationConfig* locationConfig = buildLocationConfig(child);
			serverConfig->addLocation(locationConfig);
		}
		else if (child->getStatementType() == Statement::DIRECTIVE)
			serverConfig->setDirective(child);
	}
	serverConfig->makeLocationsInhertiance();
	return (serverConfig);
}

LocationConfig*	Builder::buildLocationConfig(Statement *statement)
{
	LocationConfig* locationConfig = new LocationConfig(statement);

	const std::vector<Statement*>&	locationChildren = statement->getChildren();
	for (size_t i = 0; i < locationChildren.size(); i++)
	{
		Statement*	child = locationChildren[i];
		if (child->getStatementType() == Statement::BLOCK)
		{
			error(child, "Unexpected block inside location block");
			continue ;
		}
		locationConfig->setDirective(child);
	}
	return (locationConfig);
}

int		Builder::getError(void) const { return _error; }
void	Builder::setError(int error) { _error = error; }

int		Builder::error(Statement* statement, const TStr& msg)
{
	Console::configLog(Console::ERROR, statement->getLine(), statement->getColumn(), statement->getName(), statement->getArgs(), "BUILDING", msg);
	setError(1);
	return (_error);
}


Builder::Builder(const std::vector<Statement*>& statements) : _statements(statements) { parsingToBuild(); }

Builder::~Builder(void)
{
	for (size_t i = 0; i < _build.size(); i++)
		delete _build[i];
	_build.clear();
}

void	Builder::printBuild(void) const
{
	for (size_t i = 0; i < _build.size(); i++)
		_build[i]->print(std::cout, 0);
}

void	Builder::throwInvalid(void) const
{
	if (_error)
		throw std::runtime_error("Building failed");
}
