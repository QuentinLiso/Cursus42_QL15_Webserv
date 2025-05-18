/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommonConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:24:03 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 18:47:58 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	COMMONCONFIG_HPP
# define COMMONCONFIG_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "DataStructures.hpp"
# include "Utils.hpp"

class	CommonConfig
{
private:
	std::string					_root;					// Allowed in both server and location block
	std::vector<std::string>	_index;					// Allowed in both server and location block
	AutoIndexSettings			_autoindex;				// Allowed in both server and location block
	std::map<int, std::string>	_errorPages;			// Allowed in both server and location block
	ClientMaxBodySize			_clientMaxBodySize;		// Allowed in both server and location block
	bool						_valid;

public:
	CommonConfig(void);
	~CommonConfig(void);

	CommonConfig& inherit(const CommonConfig& other);
	bool	setRoot(const Directive* directive, const std::string& alias = "");
	bool	setIndex(const Directive* directive);
	bool	setAutoIndex(const Directive* directive);
	bool	checkValidErrorUri(const Directive* directive, const std::string& uri);
	bool	setErrorPages(const Directive* directive);
	bool	setClientMaxBodySize(const Directive* directive);

	void	setRoot(const std::string& root);
	void	setIndex(const std::vector<std::string>& index);
	void	setAutoindex(const AutoIndexSettings& autoindex);
	void	setErrorPages(const std::map<int, std::string>& errorPages);
	void	setClientMaxBodySize(const ClientMaxBodySize& clientMaxBodySize);

	const std::string&					root(void) const;
	const std::vector<std::string>& 	index(void) const;
	const AutoIndexSettings				autoindex(void) const;
	const std::map<int, std::string>& 	errorPages(void) const;
	const ClientMaxBodySize 			clientMaxBodySize(void) const;

	bool	valid(void) const;
	void	setValid(bool valid);

};



#endif