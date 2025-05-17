/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:22:48 by qliso             #+#    #+#             */
/*   Updated: 2025/05/17 23:33:07 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVERCONFIG_HPP
# define SERVERCONFIG_HPP

# include "Includes.hpp"
# include "DataStructures.hpp"
# include "Utils.hpp"
# include "CommonConfig.hpp"
# include "LocationConfig.hpp"

class 	LocationConfig;

class	ServerConfig
{
private:
	CommonConfig				_common;
	std::string					_host;						// Server block specific
	Port						_port;						// Server block specific
	std::vector<std::string>	_serverNames;				// Server block specific
	std::vector<LocationConfig>	_locations;					// Server block specific
		
	u_short		isValidPort(const std::string& port)
	{
		unsigned short	portValue;
		bool			isValid = strToVal<unsigned short>(port, portValue);
		
		if (!isValid || portValue == 0)
			throw std::runtime_error("Invalid port : '" + port + "', port must be a value between 1 and 65535");
		return (portValue);
	}
	
	std::string	isValidIp(const std::string& ip)
	{
		if (ip == "localhost")
			return ("127.0.0.1");
		std::vector<std::string>	fields = split(ip, ".");
		if (fields.size() != 4)
			throw std::runtime_error("Invalid IP address : " + ip);
		for (size_t	i = 0; i < 4; i++)
		{
			unsigned char	val;
			if (strToVal<u_char>(fields[i], val) || val == 0)
				throw std::runtime_error("Invalid IP address : " + ip);
		}
		return (ip);
	}

	void	setHostAndPort(const Directive* directive)
	{
		if (directive->arguments.size() != 1 || _port.isSet == true)
			throw std::runtime_error("Host error or doublon listen");
		
		std::string	listenArg = directive->arguments[0];
		std::size_t	sep = listenArg.find(':');

		if (sep == std::string::npos)
		{
			_host = "0.0.0.0";
			_port.isSet = true;
			_port.value = isValidPort(listenArg);
		}
		else
		{
			_host = isValidIp(listenArg.substr(0, sep));
			_port.isSet = true;
			_port = isValidPort(listenArg.substr(sep + 1));
		}
	}

	void	setServerNames(const Directive* directive)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() == 0)
			throw std::runtime_error("Missing arguments in server_name directive");
		if (_serverNames.size() != 0)
		{
			std::cout << "WARNING : server_name already populated earlier, overwriting with current server_name" << std::endl;
			_serverNames.clear();
		}
		for (size_t i = 0; i < args.size(); i++)
		{
			if (!args[i].empty())
				_serverNames.push_back(args[i]);
		}
	}
	
	void	parseServerDirective(const Directive* directive)
	{
		if (directive->name == "listen")
			setHostAndPort(directive);
		else if (directive->name == "server_name")
			setServerNames(directive);
		else if (directive->name == "root")
			_common.setRoot(directive);
		else if (directive->name == "index")
			_common.setIndex(directive);
		else if (directive->name == "autoindex")
			_common.setAutoIndex(directive);
		else if (directive->name == "error_page")
			_common.setErrorPages(directive);
		else if (directive->name == "client_max_body_size")
			_common.setClientMaxBodySize(directive);
		else
			throw std::runtime_error("Unrecognized directive '" + directive->name + "' in server block");
		// std::cout << "Directive " + directive->name + " added to the server config" << std::endl;
	}
	
	void	checkEmptyRootNoDefault(void)
	{
		const LocationConfig*	defaultLocationBlock = getDefaultLocationBlock();
		if (defaultLocationBlock == NULL)
			_locations.push_back(LocationConfig(*this));
		else if (defaultLocationBlock->common().root().empty() && defaultLocationBlock->alias().empty())
			throw std::runtime_error("Server with empty root has a default 'location /' path with empty root and alias");
	}

	void	checkDuplicatePaths(void) const
	{
		std::set<std::string>	paths;
		
		for (size_t j = 0; j < _locations.size(); j++)
		{
			const std::string& path = _locations[j].path();
			if (paths.insert(path).second == false)
				throw std::runtime_error("Duplicate location path '" + path + "' found in same server block");
		}
	}


public:
	ServerConfig(const Block* block)
	{
		parseServerBlock(block);
		checkDuplicatePaths();
		checkEmptyRootNoDefault();
	}
	
	virtual ~ServerConfig(void) {}

	void	parseServerBlock(const Block* block)
	{
		if (block->arguments.size() != 0)
			throw std::runtime_error("Server block cannot have arguments");
		for (size_t i = 0; i < block->children.size(); i++)
		{
			AConfigNode*	child = block->children[i];
			if (child->isBlock())
			{
				Block*	childBlock = dynamic_cast<Block*>(child);
				if (!childBlock)
					throw std::runtime_error("Dynamic cast childBlock failed");
				if (childBlock->name != "location")
					throw std::runtime_error("Unrecognized location block inside server");
				_locations.push_back(LocationConfig(childBlock, *this));
				continue ;
			}
			Directive*	childDirective = dynamic_cast<Directive*>(child);
			if (!childDirective)
				throw std::runtime_error("Dynamic cast childDirective failed");
			parseServerDirective(childDirective);
		}
	}
		
	const LocationConfig*	getDefaultLocationBlock(void) const
	{
		for (size_t i = 0; i < _locations.size(); i++)
		{
			if (_locations[i].path() == "/")
				return (&_locations[i]);
		}
		return (NULL);
	}

	const CommonConfig&					common(void) const { return _common; }
	const std::string&					host(void) const { return _host; }
	const Port&							port(void) const { return _port; }
	const std::vector<std::string>&		serverNames(void) const { return _serverNames; }
	const std::vector<LocationConfig>&	locations(void) const { return _locations; }

	void	printConfig(size_t i = 0) const
	{
		std::cout 	<< "****** SERVER BLOCK " << i << "******\n" 
					<< "Host : " + _host + "\n"
					<< "Port : " << _port.value << "\n"
					<< "Server name : ";
		for (size_t i = 0; i < _serverNames.size(); i++)
			std::cout << _serverNames[i] << " ";
		std::cout 	<< "\n"
					<< "Root : " + _common.root() + "\n"
					<< "Index : ";
		for (size_t i = 0; i < _common.index().size(); i++)
			std::cout << _common.index()[i] << " ";
		std::cout 	<< "\n"
				 	<< "Autoindex : " << ((_common.autoindex().value == true) ? "on" : "off") << "\n"
					<< "Error pages :\n";
		for (std::map<int, std::string>::const_iterator it = _common.errorPages().begin(); it != _common.errorPages().end(); it++)
			std::cout << "\t" << it->first << " : " << it->second << "\n";
		std::cout 	<< "Client max body size (in bytes) : " << _common.clientMaxBodySize().value << "\n";
		for (size_t i = 0; i < _locations.size(); i++)
			_locations[i].printConfig(i);
		std::cout	<< "\n" << std::endl;
	}	
};


#endif