/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ConfigBuilder.hpp                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:32:29 by qliso             #+#    #+#             */
/*   Updated: 2025/05/19 00:38:51 by qliso            ###   ########.fr       */
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
	bool						_valid;

	void	loadFromAst(const std::vector<AConfigNode*>& ast);
	void	validateIpPortDomains(void);
	void	checkZeroPorts(void);

	bool	error(const Directive* directive, const std::string& msg);
	bool	error(const Block* block, const std::string& msg);
	bool	error(const std::string& msg);
	void	warning(const Directive* directive, const std::string& msg);
	void	warning(const Block* block, const std::string& msg);
	void	warning(const std::string& msg);

public:

	ConfigBuilder(const std::vector<AConfigNode*>& ast);
	virtual ~ConfigBuilder(void);

	const std::vector<ServerConfig>&		getServers(void) const;
	void	checkValid(void) const;

	const std::vector<const ServerConfig*>	findConfigs(u_short port) const;
	const std::vector<const ServerConfig*>	findConfigs(const std::string& ip, bool exactMatch = true) const;
	const std::vector<const ServerConfig*>	findConfigs(const std::string& ip, u_short port, bool exactMatch = true) const;
	const std::vector<const ServerConfig*>	findOtherConfigs(u_short port) const;
	const std::vector<const ServerConfig*>	findOtherConfigs(const std::string& ip, bool exactMatch = true) const;
	const std::vector<const ServerConfig*>	findOtherConfigs(const std::string& ip, int port, bool exactMatch = true) const;
	void									printConfig(void) const;
	void									printConfig(const std::vector<ServerConfig>& servers) const;
	void									printConfig(const std::vector<const ServerConfig*> servers) const;
	void									printConfig(const std::string& ip, u_short port, const std::string& domain) const;
	void									printConfig(const std::string& ip, u_short port, const std::string& domain, const std::string& uri) const;
	const	ServerConfig*					findServerConfig(const std::string& ip, u_short port, const std::string& domain) const;
	const	LocationConfig*					findLocationConfig(const std::string& ip, u_short port, const std::string& domain, const std::string& uri) const;

};


#endif