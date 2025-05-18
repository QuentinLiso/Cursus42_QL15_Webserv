/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.cpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:32:29 by qliso             #+#    #+#             */
/*   Updated: 2025/05/19 00:41:35 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "ConfigBuilder.hpp"

// PRIVATE

void	ConfigBuilder::loadFromAst(const std::vector<AConfigNode*>& ast)
{
	for (std::vector<AConfigNode*>::const_iterator it = ast.begin(); it != ast.end(); it++)
	{
		if ((*it)->isBlock())
		{
			Block*	block = dynamic_cast<Block*>(*it);
			if (!block)
			{
				_valid = error("Dynamic cast to block of '" + (*it)->name + "' failed");
				continue ;
			}
			if (block->name != "server")
			{
				_valid = error(block, "Expected 'server' block at top level");
				continue ;
			}
			_servers.push_back(ServerConfig(block));
			continue ;
		}
		_valid = error("Unexpected non-block node at top level");
	}
}

void	ConfigBuilder::validateIpPortDomains(void)
{
	IpPortDomainsMap	ipPortDomainsMap;			// std::map<std::pair<std::string, int>, std::set<std::string> >		=	map < <ip, port>, domains >
		
	for (size_t i = 0; i < _servers.size(); i++)
	{
		const ServerConfig&	server = _servers[i];			
		std::pair<std::string, u_short> ipPort(server.host(), server.port().value);			// Create a pair with the server host and server port
		std::set<std::string>&	ipPortDomains = ipPortDomainsMap[ipPort];		// Look for the vector of domains at the key ip/port inside the ipPortDomainMap
			
		for (size_t j = 0; j < server.serverNames().size(); j++)
		{
			const std::string&	serverName = server.serverNames()[j];
			if (ipPortDomains.insert(serverName).second == false)				// Throw if the given serverName is found in the vector of domains at the key ip/port
				_valid = error("Duplicate server name " + serverName + " for host " + server.host() + ":" + convToStr(server.port().value));
		}

		if (server.serverNames().empty() && !ipPortDomains.empty())
			_valid = error("Server block with no server_name directive for host " + server.host() + ":" + convToStr(server.port().value) + " must be declared first in config file");
	}
}

void	ConfigBuilder::checkZeroPorts(void)
{
	const std::vector<const ServerConfig*> zerosConfigs = findConfigs("0.0.0.0");
	if (zerosConfigs.empty())
		return ;

	std::set<u_short>	zeroPorts;
	for (size_t i = 0; i < zerosConfigs.size(); i++)
	{
		if (zeroPorts.insert(zerosConfigs[i]->port().value).second == false)
			_valid = error("Duplicate listening port " + convToStr(zerosConfigs[i]->port().value) + " on default ip 0.0.0.0");
	}
	
	const std::vector<const ServerConfig*> nonZerosConfigs = findOtherConfigs("0.0.0.0");
	for (size_t i = 0; i < nonZerosConfigs.size(); i++)
	{
		const ServerConfig* config = nonZerosConfigs[i];
		if (zeroPorts.find(config->port().value) != zeroPorts.end())
			_valid = error("Duplicate listening port " + convToStr(config->port().value) + " on default ip 0.0.0.0 and ip " + config->host());
	}
}

bool	ConfigBuilder::error(const Directive* directive, const std::string& msg)
{
	return (Console::error(directive, "CONFIG BUILDER", msg));
}

bool	ConfigBuilder::error(const Block* block, const std::string& msg)
{
	return (Console::error(block, "CONFIG BUILDER", msg));
}

bool	ConfigBuilder::error(const std::string& msg)
{
	return (Console::configLog(Console::ERROR, 0, 0, "CONFIG BUILDER", msg, ""));
}

void	ConfigBuilder::warning(const Directive* directive, const std::string& msg)
{
	Console::warning(directive, "CONFIG BUILDER", msg);
}

void	ConfigBuilder::warning(const Block* block, const std::string& msg)
{
	Console::warning(block, "CONFIG BUILDER", msg);
}

void	ConfigBuilder::warning(const std::string& msg)
{
	Console::configLog(Console::WARNING, 0, 0, "CONFIG BUILDER", msg, "");
}




// PUBLIC

ConfigBuilder::ConfigBuilder(const std::vector<AConfigNode*>& ast)
{
	loadFromAst(ast);
	validateIpPortDomains();
	checkZeroPorts();
}

ConfigBuilder::~ConfigBuilder(void) {}

const std::vector<ServerConfig>&	ConfigBuilder::getServers(void) const { return (_servers); }

void	ConfigBuilder::checkValid(void) const
{
	for (size_t i = 0; i < _servers.size(); i++)
	{
		const ServerConfig& server = _servers[i];
		if(server.common().valid() == false)
			throw std::runtime_error("Config Builder failed");
		for (size_t j = 0; j < server.locations().size(); j++)
		{
			if (server.locations()[j].common().valid() == false)
				throw std::runtime_error("Config Builder failed");
		}
	}
}

const std::vector<const ServerConfig*>	ConfigBuilder::findConfigs(u_short port) const
{
	std::vector<const ServerConfig*>	configs;

	for (size_t i = 0; i < _servers.size(); i++)
	{
		const ServerConfig& server = _servers[i];
		if (server.port().value == port)
			configs.push_back(&server);
	}
	return (configs);
}

const std::vector<const ServerConfig*>	ConfigBuilder::findConfigs(const std::string& ip, bool exactMatch) const
{
	std::vector<const ServerConfig*>	configs;

	if (exactMatch)
	{
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.host() == ip)
				configs.push_back(&server);
		}
	}
	else
	{
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.host().find(ip) != std::string::npos)
				configs.push_back(&server);
		}
	}
	return (configs);
}

const std::vector<const ServerConfig*>	ConfigBuilder::findConfigs(const std::string& ip, u_short port, bool exactMatch) const
{
	std::vector<const ServerConfig*>	configs;

	if (exactMatch)
	{
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.host() == ip && server.port().value == port)
				configs.push_back(&server);
		}
	}
	else
	{
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.host().find(ip) != std::string::npos && server.port().value == port)
				configs.push_back(&server);
		}
	}
	return (configs);
}

const std::vector<const ServerConfig*>	ConfigBuilder::findOtherConfigs(u_short port) const
{
	std::vector<const ServerConfig*>	configs;

	for (size_t i = 0; i < _servers.size(); i++)
	{
		const ServerConfig& server = _servers[i];
		if (server.port().value != port)
			configs.push_back(&server);
	}
	return (configs);
}

const std::vector<const ServerConfig*>	ConfigBuilder::findOtherConfigs(const std::string& ip, bool exactMatch) const
{
	std::vector<const ServerConfig*>	configs;

	if (exactMatch)
	{
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.host() != ip)
				configs.push_back(&server);
		}
	}
	else
	{
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.host().find(ip) == std::string::npos)
				configs.push_back(&server);
		}
	}
	return (configs);
}

const std::vector<const ServerConfig*>	ConfigBuilder::findOtherConfigs(const std::string& ip, int port, bool exactMatch) const
{
	std::vector<const ServerConfig*>	configs;

	if (exactMatch)
	{
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.host() != ip || server.port().value != port)
				configs.push_back(&server);
		}
	}
	else
	{
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.host().find(ip) != std::string::npos || server.port().value != port)
				configs.push_back(&server);
		}
	}
	return (configs);
}

void	ConfigBuilder::printConfig(void) const
{
	for (size_t i = 0; i < _servers.size(); i++)
		_servers[i].printConfig(i);
}

void	ConfigBuilder::printConfig(const std::vector<ServerConfig>& servers) const
{
	for (size_t i = 0; i < servers.size(); i++)
		servers[i].printConfig(i);
}

void	ConfigBuilder::printConfig(const std::vector<const ServerConfig*> servers) const
{
	for (size_t i = 0; i < servers.size(); i++)
	{
		if (servers[i])
			servers[i]->printConfig(i);
	}
}

void	ConfigBuilder::printConfig(const std::string& ip, u_short port, const std::string& domain) const
{
	const ServerConfig*	serverConfig = findServerConfig(ip, port, domain);
	if (serverConfig)
		serverConfig->printConfig();
	else
		std::cout << "No config found for ip '" << ip << "', port '" << port << "', domain '" << domain << "'" << std::endl;
}

void	ConfigBuilder::printConfig(const std::string& ip, u_short port, const std::string& domain, const std::string& uri) const
{
	const LocationConfig*	locConfig = findLocationConfig(ip, port, domain, uri);
	if (locConfig)
		locConfig->printConfig();
	else
		std::cout << "No config found for ip '" << ip << "', port '" << port << "', domain '" << domain << "', uri '" << uri << "'" << std::endl;
}

const	ServerConfig*	ConfigBuilder::findServerConfig(const std::string& ip, u_short port, const std::string& domain) const
{
	std::vector<const ServerConfig*>	configs = findConfigs(ip, port);

	if (configs.empty())			// NO config for this ip/port
		return (NULL);
	if (domain.empty())				// There are some config but domain is empty so we look for a default config, which can only be the first one
		return (configs[0]);

	for (size_t i = 0; i < configs.size(); i++)
	{
		const std::vector<std::string>& configServerNames = configs[i]->serverNames();
		if (std::find(configServerNames.begin(), configServerNames.end(), domain) != configServerNames.end())
			return (configs[i]);	// We found the domain in the vector of serverNames for a given ip/port (uniqueness was checked during validation)
	}
	return (configs[0]);			// Fallback to first config if no domain match but we had found config(s) for this ip/port
}

const	LocationConfig*	ConfigBuilder::findLocationConfig(const std::string& ip, u_short port, const std::string& domain, const std::string& uri) const
{
	const ServerConfig*	serverConfig = findServerConfig(ip, port, domain);
	if (!serverConfig || uri.empty() || uri[0] != '/')
		return (NULL);
		
	const std::vector<LocationConfig>& locations = serverConfig->locations();
	const LocationConfig* bestMatch = serverConfig->getDefaultLocationBlock();
	if (!bestMatch)
		return (NULL);

	for (size_t i = 0; i < locations.size(); i++)
	{
		const std::string& path = locations[i].path();
		if (uri.compare(0, path.size(), path) == 0 && path.size() > bestMatch->path().size())
			bestMatch = &locations[i];
	}
	return (bestMatch);
}
