/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ServerConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:22:48 by qliso             #+#    #+#             */
/*   Updated: 2025/05/19 00:17:32 by qliso            ###   ########.fr       */
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
		
	void	parseServerDirective(const Directive* directive);
	
	bool	setHostAndPort(const Directive* directive);
	bool	isValidPort(const Directive* directive, const std::string& arg);
	bool	isValidIp(const Directive* directive, const std::string& arg);
	
	bool	setServerNames(const Directive* directive);
	
	void	checkPort(void);
	void	checkDuplicatePaths(void);
	void	checkEmptyRootNoDefault(void);

	bool	error(const Directive* directive, const std::string& msg);
	bool	error(const Block* block, const std::string& msg);
	bool	error(const std::string& msg);
	void	warning(const Directive* directive, const std::string& msg);
	void	warning(const Block* block, const std::string& msg);
	void	warning(const std::string& msg);

public:
	ServerConfig(const Block* block);
	virtual ~ServerConfig(void);
	void	parseServerBlock(const Block* block);
	const LocationConfig*	getDefaultLocationBlock(void) const;

	const CommonConfig&					common(void) const;
	const std::string&					host(void) const;
	const Port&							port(void) const;
	const std::vector<std::string>&		serverNames(void) const;
	const std::vector<LocationConfig>&	locations(void) const;

	void	printConfig(size_t i = 0) const;
};


#endif