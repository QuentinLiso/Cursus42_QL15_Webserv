/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommonConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:24:03 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 18:48:02 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "CommonConfig.hpp"

CommonConfig::CommonConfig(void) : _root(""), _autoindex(), _clientMaxBodySize(), _valid(true) {}
CommonConfig::~CommonConfig(void) {}

CommonConfig& CommonConfig::inherit(const CommonConfig& other)
{
	if (this == &other)
		return (*this);
	
	if (_root.empty())
		_root = other._root;
	if (_index.empty())
		_index = other._index;
	if (_autoindex.isSet == false)
		_autoindex = other._autoindex;
	if (_errorPages.empty())
		_errorPages = other._errorPages;
	if (_clientMaxBodySize.isSet == false)
		_clientMaxBodySize = other._clientMaxBodySize;
	return (*this);
}

bool	CommonConfig::setRoot(const Directive* directive, const std::string& alias)
{
	std::vector<std::string>	args = directive->arguments;
	if (args.size() != 1 || args[0].empty())
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Missing or invalid argument in root directive ", args));
	if (!alias.empty())
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Cannot have both an alias and a root directive ", args[0]));

	std::string	root = args[0];
	if (root[0] != '/')
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "root directive must start with a '/' :", root));
	if (!isValidFilepath(root))
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Invalid character found in root directive", root));
	if (!_root.empty())
		Console::configLog(Console::WARNING, directive->line, directive->line, "COMMON CONFIG", "root directive already defined earlier, overwriting with current root directive", root);
	normalizeFilepath(root);
	_root = root;
	return (true);
}

bool	CommonConfig::setIndex(const Directive* directive)
{
	std::vector<std::string>	args = directive->arguments;
	if (args.size() == 0)
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Missing arguments in index directive ", ""));
	if (_index.size() != 0)
	{
		Console::configLog(Console::WARNING, directive->line, directive->line, "COMMON CONFIG", "index directive already defined earlier, overwriting with current index directive", args);
		_index.clear();
	}
	for (size_t i = 0; i < args.size(); i++)
	{
		std::string	fileName = args[i];
		if (fileName.empty() || fileName.find('/') != std::string::npos || fileName == "." || fileName == "..")
		{
			Console::configLog(Console::WARNING, directive->line, directive->line, "COMMON CONFIG", "invalid filename in index directive", fileName);
			continue ;
		}
		_index.push_back(args[i]);
	}
	return (true);
}

bool	CommonConfig::setAutoIndex(const Directive* directive)
{
	std::vector<std::string>	args = directive->arguments;
	if (args.size() != 1)
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Missing arguments in autoindex directive ", ""));
	if (_autoindex.isSet == true)
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Duplicate autoindex directive found", args[0]));
	if (args[0] == "on")
	{
		_autoindex.isSet = true;
		_autoindex.value = true;
		return (true);
	}
	else if (args[0] == "off")
	{
		_autoindex.isSet = true;
		_autoindex.value = false;
		return (true);
	}
	else
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Invalid arguments in autoindex directive : only 'on' and 'off' allowed, and we got", args[0]));
}

bool	CommonConfig::checkValidErrorUri(const Directive* directive, const std::string& uri)
{
	std::string	http = "http://";
	std::string https = "https://";
	
	if (uri.empty())
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "URI cannot be empty in error_pages directive", directive->arguments));
	if (uri[0] == '/' && uri.size() > 1)
	{
		if (containsDoubleDotsAccess(uri))
			return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "URI cannot contain double dots access in error_pages directive", directive->arguments));
		return (true);
	}
	if ((!uri.compare(0, http.size(), http) && (uri.size() > http.size()))
		|| (!uri.compare(0, https.size(), https) && (uri.size() > https.size())))
		return (true);
	return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Invalid URI in error_pages directive", directive->arguments));
}

bool	CommonConfig::setErrorPages(const Directive* directive)
{
	std::vector<std::string>	args = directive->arguments;
		
	if (args.size() < 2)
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Missing arguments in error_pages directive", args));
		
	size_t	last = args.size() - 1;
	if (checkValidErrorUri(directive, args[last]) == false)
		return (false);
	for (size_t i = 0; i < last; i++)
	{
		u_int	error;
		if (strToVal<u_int>(args[i], error) == false || error < 400 || error > 599)
			return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Invalid error code in error_pages directive", args));		
		if (_errorPages.find(error) != _errorPages.end())
			return (Console::configLog(Console::WARNING, directive->line, directive->column, "COMMON CONFIG", "Error code already mapped earlier in error_pages, overwritten by current directive", args[i]));	
		_errorPages[error] = args[last];
	}
	return (true);
}

bool	CommonConfig::setClientMaxBodySize(const Directive* directive)
{
	std::vector<std::string>	args = directive->arguments;

	if (args.size() != 1)
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Missing arguments in client_max_body_size directive", args));		
	if (_clientMaxBodySize.isSet == true)
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Duplicate client_max_body_size directive", args[0]));		
	size_t	bytes;
	if (strToBytes(args[0], bytes) == false)
		return (Console::configLog(Console::ERROR, directive->line, directive->column, "COMMON CONFIG", "Invalid argument in client_max_body_size directive", args[0]));		
	_clientMaxBodySize.isSet = true;
	_clientMaxBodySize.value = bytes;
	return (true);
}

void	CommonConfig::setRoot(const std::string& root) { _root = root; }
void	CommonConfig::setIndex(const std::vector<std::string>& index) { _index = index; }
void	CommonConfig::setAutoindex(const AutoIndexSettings& autoindex) { _autoindex = autoindex; }
void	CommonConfig::setErrorPages(const std::map<int, std::string>& errorPages) { _errorPages = errorPages; }
void	CommonConfig::setClientMaxBodySize(const ClientMaxBodySize& clientMaxBodySize) { _clientMaxBodySize = clientMaxBodySize; }

const std::string&					CommonConfig::root(void) const { return _root; }
const std::vector<std::string>& 	CommonConfig::index(void) const { return _index; }
const AutoIndexSettings				CommonConfig::autoindex(void) const { return _autoindex; }
const std::map<int, std::string>& 	CommonConfig::errorPages(void) const { return _errorPages; }
const ClientMaxBodySize 			CommonConfig::clientMaxBodySize(void) const { return _clientMaxBodySize; }

bool	CommonConfig::valid(void) const { return _valid; }
void	CommonConfig::setValid(bool valid) { _valid = valid; }