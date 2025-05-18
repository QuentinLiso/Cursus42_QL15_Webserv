/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:22:48 by qliso             #+#    #+#             */
/*   Updated: 2025/05/19 00:19:15 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ServerConfig.hpp"

// PRIVATE

void	ServerConfig::parseServerDirective(const Directive* directive)
{
	bool	valid;
	
	if (directive->name == "listen")
		valid = setHostAndPort(directive);
	else if (directive->name == "server_name")
		valid = setServerNames(directive);
	else if (directive->name == "root")
		valid = _common.setRoot(directive);
	else if (directive->name == "index")
		valid = _common.setIndex(directive);
	else if (directive->name == "autoindex")
		valid = _common.setAutoIndex(directive);
	else if (directive->name == "error_page")
		valid = _common.setErrorPages(directive);
	else if (directive->name == "client_max_body_size")
		valid = _common.setClientMaxBodySize(directive);
	else
	{
		warning(directive, "Directive in server block is not handled by webserv 42");
		return ;
	}
	if (_common.valid() == true && valid == false)
		_common.setValid(false);
}

bool	ServerConfig::setHostAndPort(const Directive* directive)
{
	if (directive->arguments.size() != 1 || _port.isSet == true)
		return (error("Invalid number of arguments for listen directive"));
	if (_port.isSet == true)
		return (error("Duplicate listen directive inside the same server block"));
		
	std::string	listenArg = directive->arguments[0];
	std::size_t	sep = listenArg.find(':');

	if (sep == std::string::npos)
	{
		warning(directive, "Server listen directive has no IP address defined, adding '0.0.0.0' by default");
		_host = "0.0.0.0";
		return (isValidPort(directive, listenArg));
	}
	else
	{
		bool	validIp = isValidIp(directive, listenArg.substr(0, sep));
		bool	validPort = isValidPort(directive, listenArg.substr(sep + 1));
		return (validIp && validPort);
	}
}

bool	ServerConfig::isValidPort(const Directive* directive, const std::string& arg)
{
	unsigned short	portValue;
	bool			isValid = strToVal<unsigned short>(arg, portValue);
	
	if (!isValid || portValue == 0)
		return (error(directive, "Invalid port, port must be a value between 1 and 65535"));
	_port.isSet = true;
	_port.value = portValue;
	return (true);
}
	
bool	ServerConfig::isValidIp(const Directive* directive, const std::string& ip)
{
	if (ip == "localhost")
	{
		_host = "127.0.0.1";
		return (true);
	}
		
	std::vector<std::string>	fields = split(ip, ".");
	if (fields.size() != 4)
		return (error(directive, "Invalid IP address"));
	for (size_t	i = 0; i < 4; i++)
	{
		unsigned char	val;
		if (strToVal<u_char>(fields[i], val) == false)
			return (error(directive, "Invalid IP address"));
	}
	_host = ip;
	return (true);
}


bool	ServerConfig::setServerNames(const Directive* directive)
{
	std::vector<std::string>	args = directive->arguments;
	if (args.size() == 0)
		return(error(directive, "Missing arguments in server_name directive"));
	if (_serverNames.size() != 0)
	{
		warning(directive, "server_name already populated earlier, overwriting with current server_name directive");
		_serverNames.clear();
	}
	for (size_t i = 0; i < args.size(); i++)
	{
		if (!args[i].empty())
			_serverNames.push_back(args[i]);
	}
	return (true);
}

void	ServerConfig::checkPort(void)
{
		if (_port.isSet == false)
			_common.setValid(error("Server block has no defined port"));
}

void	ServerConfig::checkDuplicatePaths(void)	
{
	std::set<std::string>	paths;
	
	for (size_t j = 0; j < _locations.size(); j++)
	{
		const std::string& path = _locations[j].path();
		if (paths.insert(path).second == false)
			_common.setValid(error("Duplicate location path '" + path + "' found in the same server block"));
	}
}

void	ServerConfig::checkEmptyRootNoDefault(void)
{
	const LocationConfig*	defaultLocationBlock = getDefaultLocationBlock();
	if (defaultLocationBlock == NULL)
		_locations.push_back(LocationConfig(*this));
	else if (defaultLocationBlock->common().root().empty() && defaultLocationBlock->alias().empty())
		_common.setValid(error("Server with empty root has default 'location /' path with empty root and alias"));
}


bool	ServerConfig::error(const Directive* directive, const std::string& msg)
{
	return (Console::error(directive, "SERVER CONFIG", msg));
}

bool	ServerConfig::error(const Block* block, const std::string& msg)
{
	return (Console::error(block, "SERVER CONFIG", msg));
}

bool	ServerConfig::error(const std::string& msg)
{
	return (Console::configLog(Console::ERROR, 0, 0, "SERVER CONFIG", msg, ""));
}

void	ServerConfig::warning(const Directive* directive, const std::string& msg)
{
	Console::warning(directive, "SERVER CONFIG", msg);
}

void	ServerConfig::warning(const Block* block, const std::string& msg)
{
	Console::warning(block, "SERVER CONFIG", msg);
}

void	ServerConfig::warning(const std::string& msg)
{
	Console::configLog(Console::WARNING, 0, 0, "SERVER CONFIG", msg, "");
}



// PUBLIC

ServerConfig::ServerConfig(const Block* block)
{
	parseServerBlock(block);
	checkPort();
	checkDuplicatePaths();
	checkEmptyRootNoDefault();
}
	
ServerConfig::~ServerConfig(void) {}

void	ServerConfig::parseServerBlock(const Block* block)
{
	if (block->arguments.size() != 0)
		_common.setValid(error("Server block cannot have arguments"));
	for (size_t i = 0; i < block->children.size(); i++)
	{
		AConfigNode*	child = block->children[i];
		if (child->isBlock())
		{
			Block*	childBlock = dynamic_cast<Block*>(child);
			if (!childBlock)
			{
				_common.setValid(error("Dynamic cast of '" + child->name + "' to childBlock failed"));
				continue ;
			}
			if (childBlock->name != "location")
			{
				_common.setValid(error(childBlock, "Unrecognized location block inside server"));
				continue ;
			}
			_locations.push_back(LocationConfig(childBlock, *this));
			continue ;
		}
		Directive*	childDirective = dynamic_cast<Directive*>(child);
		if (!childDirective)
		{
			_common.setValid(error("Dynamic cast of '" + child->name + "' to childDirective failed"));
			continue ;
		}
		parseServerDirective(childDirective);
	}
}
		
const LocationConfig*	ServerConfig::getDefaultLocationBlock(void) const
{
	for (size_t i = 0; i < _locations.size(); i++)
	{
		if (_locations[i].path() == "/")
			return (&_locations[i]);
	}
	return (NULL);
}

const CommonConfig&					ServerConfig::common(void) const { return _common; }
const std::string&					ServerConfig::host(void) const { return _host; }
const Port&							ServerConfig::port(void) const { return _port; }
const std::vector<std::string>&		ServerConfig::serverNames(void) const { return _serverNames; }
const std::vector<LocationConfig>&	ServerConfig::locations(void) const { return _locations; }

void	ServerConfig::printConfig(size_t i) const
{
	std::cout 	<< "****** SERVER BLOCK " << i << "******\n" 
				<< "Host : " + _host + "\n"
				<< "Port : " << _port.value << "\n"
				<< "Server name : ";
	for (size_t j = 0; j < _serverNames.size(); j++)
		std::cout << _serverNames[j] << " ";
	std::cout 	<< "\n"
				<< "Root : " + _common.root() + "\n"
				<< "Index : ";
	for (size_t j = 0; j < _common.index().size(); j++)
		std::cout << _common.index()[j] << " ";
	std::cout 	<< "\n"
			 	<< "Autoindex : " << ((_common.autoindex().value == true) ? "on" : "off") << "\n"
				<< "Error pages :\n";
	for (std::map<int, std::string>::const_iterator it = _common.errorPages().begin(); it != _common.errorPages().end(); it++)
		std::cout << "\t" << it->first << " : " << it->second << "\n";
	std::cout 	<< "Client max body size (in bytes) : " << _common.clientMaxBodySize().value << "\n";
	for (size_t j = 0; j < _locations.size(); j++)
		_locations[j].printConfig(i);
	std::cout	<< "\n" << std::endl;
}
