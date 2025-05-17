/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:32:29 by qliso             #+#    #+#             */
/*   Updated: 2025/05/17 23:33:46 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	CONFIGBUILDER_HPP
# define CONFIGBUILDER_HPP

# include "Includes.hpp"
# include "DataStructures.hpp"
# include "Utils.hpp"
# include "CommonConfig.hpp"
# include "LocationConfig.hpp"
# include "ServerConfig.hpp"

class	ConfigBuilder
{
public:
	typedef std::map<std::pair<std::string, u_short>, std::set<std::string> >	IpPortDomainsMap;

private:
	std::vector<ServerConfig>	_servers;

	void	loadFromAst(const std::vector<AConfigNode*>& ast)
	{
		for (std::vector<AConfigNode*>::const_iterator it = ast.begin(); it != ast.end(); it++)
		{
			if ((*it)->isBlock())
			{
				Block*	block = dynamic_cast<Block*>(*it);
				if (!block)
					throw std::runtime_error("Dynamic cast to block failed");
				if (block->name != "server")
					throw std::runtime_error("Expected 'server' block at top level");
				_servers.push_back(ServerConfig(block));
				continue ;
			}
			throw std::runtime_error("Unrecognized server block at top ast level");
		}
	}

	void	validateIpPortDomains(void) const
	{
		IpPortDomainsMap	ipPortDomainsMap;			// std::map<std::pair<std::string, int>, std::set<std::string> >		=	map < <ip, port>, domains >
		
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig&	server = _servers[i];
			if (server.port().isSet == false)
				throw std::runtime_error("server has no defined port");
			
			std::pair<std::string, u_short> ipPort(server.host(), server.port().value);			// Create a pair with the server host and server port
			std::set<std::string>&	ipPortDomains = ipPortDomainsMap[ipPort];		// Look for the vector of domains at the key ip/port inside the ipPortDomainMap
			
			for (size_t j = 0; j < server.serverNames().size(); j++)
			{
				const std::string&	serverName = server.serverNames()[j];
				if (ipPortDomains.insert(serverName).second == false)				// Throw if the given serverName is found in the vector of domains at the key ip/port
					throw std::runtime_error("Duplicate server name " + serverName + " for host " + server.host() + ":" + convToStr(server.port().value));
			}

			if (server.serverNames().empty() && !ipPortDomains.empty())
				throw std::runtime_error("Server block with no server_name directive for host " + server.host() + ":" + convToStr(server.port().value) + " must be declared first in config file");
		}
	}

	void	checkZeroPorts(void) const
	{
		const std::vector<const ServerConfig*> zerosConfigs = findConfigs("0.0.0.0");
		if (zerosConfigs.empty())
			return ;
	
		std::set<u_short>	zeroPorts;
		for (size_t i = 0; i < zerosConfigs.size(); i++)
		{
			if (zeroPorts.insert(zerosConfigs[i]->port().value).second == false)
				throw std::runtime_error("Duplicate listening port " + convToStr(zerosConfigs[i]->port().value) + " on default ip 0.0.0.0");
		}
		
		const std::vector<const ServerConfig*> nonZerosConfigs = findOtherConfigs("0.0.0.0");
		for (size_t i = 0; i < nonZerosConfigs.size(); i++)
		{
			const ServerConfig* config = nonZerosConfigs[i];
			if (zeroPorts.find(config->port().value) != zeroPorts.end())
				throw std::runtime_error("Duplicate listening port " + convToStr(config->port().value) + " on default ip 0.0.0.0 and ip " + config->host());
		}
	}

public:

	ConfigBuilder(const std::vector<AConfigNode*>& ast)
	{
		loadFromAst(ast);
		validateIpPortDomains();
		checkZeroPorts();
	}

	~ConfigBuilder(void) {}

	const std::vector<ServerConfig>&	getServers(void) const { return (_servers); }

	const std::vector<const ServerConfig*>	findConfigs(u_short port) const
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

	const std::vector<const ServerConfig*>	findConfigs(const std::string& ip, bool exactMatch = true) const
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

	const std::vector<const ServerConfig*>	findConfigs(const std::string& ip, u_short port, bool exactMatch = true) const
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

	const std::vector<const ServerConfig*>	findOtherConfigs(u_short port) const
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

	const std::vector<const ServerConfig*>	findOtherConfigs(const std::string& ip, bool exactMatch = true) const
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

	const std::vector<const ServerConfig*>	findOtherConfigs(const std::string& ip, int port, bool exactMatch = true) const
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

	void	printConfig(const std::vector<ServerConfig>& servers) const
	{
		for (size_t i = 0; i < servers.size(); i++)
			servers[i].printConfig(i);
	}

	void	printConfig(const std::vector<const ServerConfig*> servers) const
	{
		for (size_t i = 0; i < servers.size(); i++)
		{
			if (servers[i])
				servers[i]->printConfig(i);
		}
	}

	void	printConfig(const std::string& ip, u_short port, const std::string& domain) const
	{
		const ServerConfig*	serverConfig = findServerConfig(ip, port, domain);
		if (serverConfig)
			serverConfig->printConfig();
		else
			std::cout << "No config found for ip '" << ip << "', port '" << port << "', domain '" << domain << "'" << std::endl;
	}

	void	printConfig(const std::string& ip, u_short port, const std::string& domain, const std::string& uri) const
	{
		const LocationConfig*	locConfig = findLocationConfig(ip, port, domain, uri);
		if (locConfig)
			locConfig->printConfig();
		else
			std::cout << "No config found for ip '" << ip << "', port '" << port << "', domain '" << domain << "', uri '" << uri << "'" << std::endl;
	}

	const	ServerConfig*	findServerConfig(const std::string& ip, u_short port, const std::string& domain) const
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

	const	LocationConfig*	findLocationConfig(const std::string& ip, u_short port, const std::string& domain, const std::string& uri) const
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

};


#endif