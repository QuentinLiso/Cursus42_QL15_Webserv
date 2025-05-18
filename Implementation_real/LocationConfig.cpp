/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 23:27:51 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "LocationConfig.hpp"

// PRIVATE

void	LocationConfig::parseLocationDirective(const Directive* directive)
{
	bool	valid;
	
	if(directive->name == "root")
		valid = _common.setRoot(directive, _alias);
	else if (directive->name == "index")
		valid = _common.setIndex(directive);
	else if (directive->name == "autoindex")
		valid = _common.setAutoIndex(directive);
	else if (directive->name == "error_page")
		valid = _common.setErrorPages(directive);
	else if (directive->name == "client_max_body_size")
		valid = _common.setClientMaxBodySize(directive);
	else if (directive->name == "alias")
		valid = setAlias(directive);
	else if (directive->name == "allowed_methods")
		valid = setAllowedMethods(directive);
	else if (directive->name == "cgi_pass")
		valid = setCgiPass(directive);
	else
	{
		warning(directive, "Directive in location block is not handled by webserv 42");
		return ;
	}
	if (_common.valid() == true && valid == false)
		_common.setValid(false);
}

bool	LocationConfig::setLocationPath(const Block* block)
{
	std::vector<std::string>	args = block->arguments;
	
	if (args.size() != 1 || args[0].empty())
		return (error(block, "Missing or invalid path argument in location statement"));
		
	std::string	path = args[0];
	if (path[0] != '/')
		return (error(block, "Location path argument must start with a '/':"));

	if (!isValidFilepath(path))
		return (error(block, "Invalid character found in location path"));

	removeStrDuplicateChar(path, '/');
	if (containsDoubleDotsAccess(path))
		return (error(block, "Location path contains forbidden double dots access"));

	_path = path;
	return (true);
}

bool	LocationConfig::setAlias(const Directive* directive)
{
	std::vector<std::string>	args = directive->arguments;
	if (args.size() != 1 || args[0].empty())
		return (error(directive, "Invalid character found in location path"));
	if (!_alias.empty())
		return (error(directive, "Duplicate alias directive in the same location"));
	if (!_common.root().empty())
		return (error(directive, "Cannot have both an alias and a root directive inside a location block"));
		
	std::string	alias = args[0];
	if (alias[0] != '/')
		return (error(directive, "Alias directive must start with a '/'"));
	if (!isValidFilepath(alias))
		return (error(directive, "Invalid character found in alias directive"));
	if (containsDoubleDotsAccess(alias))
		return (error(directive, "Alias cannot have '..' as path access inside the provided argument"));
	isExecutableDirectory(alias);
	_alias = alias;
	return (true);
}

	
void	LocationConfig::addAllowedMethod(const Directive* directive, std::set<HttpMethods>& configHttpMethods, HttpMethods methodId)
{
	if (configHttpMethods.insert(methodId).second == false)
		warning(directive, "Duplicate HTTP method found in allowed_methods directive");
}

bool	LocationConfig::setAllowedMethods(const Directive* directive)
{
	std::vector<std::string>	args = directive->arguments;
	
	if (args.size() == 0)
		return (error(directive, "Missing arguments in allowed_methods directive"));
	if (!_allowedMethods.empty())
		return (error(directive, "Duplicate allowed_methods directive found in location config"));
	for (size_t i = 0; i < args.size(); i++)
	{
		std::string&	method = args[i];
		if (method == "GET")
			addAllowedMethod(directive, _allowedMethods, HTTP_GET);
		else if (method == "PUT")
			addAllowedMethod(directive, _allowedMethods, HTTP_PUT);
		else if (method == "DELETE")
			addAllowedMethod(directive, _allowedMethods, HTTP_DELETE);
		else if (method == "POST")
			addAllowedMethod(directive, _allowedMethods, HTTP_POST);
		else
			return (error(directive, "Invalid argument in allowed_methods directive"));
	}
	return (true);
}

void	LocationConfig::fillEmptyAllowedMethods(void)
{
	if (_allowedMethods.empty())
	{
		warning("location '" + _path + "' has no allowed_methods directive, adding GET and POST by default");
		_allowedMethods.insert(HTTP_GET);
		_allowedMethods.insert(HTTP_POST);
	}
}

	
bool	LocationConfig::checkCgiBin(const Directive* directive, const std::string& filePath)
{
	struct stat	fileStatus;
	if (stat(filePath.c_str(), &fileStatus) != 0)
		return (error(directive, "cgi_pass file does not exist"));
	
	if (!S_ISREG(fileStatus.st_mode))
		return (error(directive, "cgi_pass file is not a file"));
		
	if (access(filePath.c_str(), X_OK))
		return (error(directive, "cgi_pass file is not executable"));
	
	return (true);
}

bool	LocationConfig::checkCgiDirectory(const Directive* directive, const std::string& filePath)
{
	struct stat dirStatus;
	std::string dir = filePath.substr(0, filePath.find_last_of('/'));
		
	if (stat(dir.c_str(), &dirStatus) == 0 && dirStatus.st_mode & S_IWOTH)
		return (error(directive, "cgi_pass file directory is world writable, unsafe for cgi_pass"));
	return (true);
}

bool	LocationConfig::setCgiPass(const Directive* directive)
{
	std::vector<std::string>	args = directive->arguments;

	if (args.size() != 1 || args[0].empty())
		return (error(directive, "cgi_pass directive should have exactly 1 non-empty argument"));
	if (!_cgiPass.empty())
		return (error(directive, "Duplicate cgi_pass directive in the same location block"));
	if (args[0][0] != '/')
		return (error(directive, "cgi_pass argument must be an absolute path and must start with '/'"));
	const std::string& cgiPass = args[0];
	if (checkCgiBin(directive, cgiPass))
		return (false);
	if (checkCgiDirectory(directive, cgiPass))
		return (false);
	_cgiPass = cgiPass;
	return (true);
}

void	LocationConfig::inheritCommonFromServerConfig(const ServerConfig& serverConfig)
{
	_common.inherit(serverConfig.common());
	if (!_alias.empty())
	{
		_common.setRoot("");
	}
	else if (_common.root().empty())
	{
		error("Empty root at server level is not covered by root or alias at location level " + _path);
		_common.setValid(false);
	}
	else
	{
		isExecutableDirectory(joinPaths(_common.root(), _path));
	}
		
}


bool	LocationConfig::error(const Directive* directive, const std::string& msg)
{
	return (Console::error(directive, "LOCATION CONFIG", msg));
}

bool	LocationConfig::error(const Block* block, const std::string& msg)
{
	return (Console::error(block, "LOCATION CONFIG", msg));
}

bool	LocationConfig::error(const std::string& msg)
{
	return (Console::configLog(Console::ERROR, 0, 0, "LOCATION CONFIG", msg, ""));
}

void	LocationConfig::warning(const Directive* directive, const std::string& msg)
{
	Console::warning(directive, "LOCATION CONFIG", msg);
}

void	LocationConfig::warning(const Block* block, const std::string& msg)
{
	Console::warning(block, "LOCATION CONFIG", msg);
}

void	LocationConfig::warning(const std::string& msg)
{
	Console::configLog(Console::WARNING, 0, 0, "LOCATION CONFIG", msg, "");
}


// PUBLIC

LocationConfig::LocationConfig(const ServerConfig& serverConfig) : _pathModifier(""), _path("/"), _alias(""), _cgiPass("")
{
	inheritCommonFromServerConfig(serverConfig);
	fillEmptyAllowedMethods();
}

LocationConfig::LocationConfig(const Block* block, const ServerConfig& serverConfig)
{
	parseLocationBlock(block, serverConfig);
	inheritCommonFromServerConfig(serverConfig);
	fillEmptyAllowedMethods();
}

LocationConfig::~LocationConfig(void) {}

const CommonConfig&		LocationConfig::common(void) const { return _common; }
const std::string&		LocationConfig::pathModifier(void) const { return _pathModifier; }
const std::string&		LocationConfig::path(void) const { return _path; }
const std::string&		LocationConfig::alias(void) const { return _alias; }
const std::set<HttpMethods>	LocationConfig::allowedMethods(void) const { return _allowedMethods; }
const std::string&		LocationConfig::cgiPass(void) const { return _cgiPass; }

void	LocationConfig::parseLocationBlock(const Block* block, const ServerConfig& serverConfig)
{
	_common.setValid(setLocationPath(block));
	for (size_t i = 0; i < block->children.size(); i++)
	{
		AConfigNode*	child = block->children[i];
		if (child->isBlock())
		{
			_common.setValid(error("Block found in location block " + child->name));
			continue ;
		}

		Directive*	childDirective = dynamic_cast<Directive*>(child);
		if (!childDirective)
		{
			_common.setValid(error("Dynamic cast childDirective failed '" + child->name + "'"));
			continue ;
		}
		parseLocationDirective(childDirective);
	}
}

void	LocationConfig::printConfig(size_t i) const
{
	std::cout 	<< "\t****** LOCATION BLOCK " << i << " ******\n" 
				<< "\tPath modifier : " + _pathModifier + "\n"
				<< "\tPath : " + _path + "\n"
				<< "\tRoot : " + _common.root() + "\n"
				<< "\tAlias : " + _alias + "\n"
				<< "\tIndex : ";
	for (size_t j = 0; j < _common.index().size(); j++)
		std::cout << _common.index()[j] << " ";
	std::cout 	<< "\n"
			 	<< "\tAutoindex : " << ((_common.autoindex().value == true) ? "on" : "off") << "\n"
				<< "\tError pages :\n";
	for (std::map<int, std::string>::const_iterator it = _common.errorPages().begin(); it != _common.errorPages().end(); it++)
		std::cout << "\t\t" << it->first << " : " << it->second << "\n";
	std::cout 	<< "\tClient max body size (in bytes) : " << _common.clientMaxBodySize().value << "\n"
				<< "\tAllowed methods (1: GET, 2: POST, 3: DELETE, 4: PUT) : ";
	for (std::set<HttpMethods>::const_iterator it = _allowedMethods.begin(); it != _allowedMethods.end(); it++)
		std::cout << (*it + 1) << " ";
	std::cout	<< "\n"
				<< "\tCGI Pass : " + _cgiPass
				<< "\n" << std::endl;
}
