/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 18:37:39 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	LOCATIONCONFIG_HPP
# define LOCATIONCONFIG_HPP

# include "Includes.hpp"
# include "DataStructures.hpp"
# include "Utils.hpp"
# include "CommonConfig.hpp"
# include "ServerConfig.hpp"

class	ServerConfig;

class	LocationConfig
{
private:
	CommonConfig				_common;				
	std::string					_pathModifier;			// Location block specific
	std::string					_path;					// Location block specific
	std::string					_alias;					// Location block specific
	std::set<HttpMethods>		_allowedMethods;		// Location block specific
	std::string					_cgiPass;				// Location block specific

	void	parseLocationDirective(const Directive* directive);
	bool	setLocationPath(const Block* block);
	bool	setAlias(const Directive* directive);
	void	addAllowedMethod(const Directive* directive, std::set<HttpMethods>& configHttpMethods, HttpMethods methodId);
	bool	setAllowedMethods(const Directive* directive);
	void	fillEmptyAllowedMethods(void);
	bool	checkCgiBin(const Directive* directive, const std::string& filePath);
	bool	checkCgiDirectory(const Directive* directive, const std::string& filePath);
	bool	setCgiPass(const Directive* directive);
	void	inheritCommonFromServerConfig(const ServerConfig& serverConfig);
	
	bool	error(const Directive* directive, const std::string& msg);
	bool	error(const Block* block, const std::string& msg);
	bool	error(const std::string& msg);

	void	warning(const Directive* directive, const std::string& msg);
	void	warning(const Block* block, const std::string& msg);
	void	warning(const std::string& msg);

public:
	LocationConfig(const ServerConfig& serverConfig);
	LocationConfig(const Block* block, const ServerConfig& serverConfig);
	~LocationConfig(void);

	const CommonConfig&		common(void) const;
	const std::string&		pathModifier(void) const;
	const std::string&		path(void) const;
	const std::string&		alias(void) const;
	const std::set<HttpMethods>	allowedMethods(void) const;
	const std::string&		cgiPass(void) const;

	void	parseLocationBlock(const Block* block, const ServerConfig& serverConfig);
	void	printConfig(size_t i = 0) const;

};



#endif