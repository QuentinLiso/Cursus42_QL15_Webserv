/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommonConfig.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:24:03 by qliso             #+#    #+#             */
/*   Updated: 2025/05/17 23:30:06 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	COMMONCONFIG_HPP
# define COMMONCONFIG_HPP

# include "Includes.hpp"
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

public:
	CommonConfig(void) : _root(""), _autoindex(), _clientMaxBodySize() {}
	~CommonConfig(void) {}

	CommonConfig& inherit(const CommonConfig& other)
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
	}

	void	setRoot(const Directive* directive, const std::string& alias = "")
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid argument in root directive");
		if (!alias.empty())
			throw std::runtime_error("Cannot have both an alias and a root directive");

		std::string	root = args[0];
		if (root[0] != '/')
			throw std::runtime_error("root directive '" + root + "' must start with a '/'");
		if (!isValidFilepath(root))
				throw std::runtime_error("Invalid character found in root directive '" + root + "'");
		if (!_root.empty())
			std::cout << "WARNING: root already defined earlier, overwriting with current directive" << std::endl;
		normalizeFilepath(root);
		_root = root;
	}

	void	setIndex(const Directive* directive)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() == 0)
			throw std::runtime_error("Missing arguments in index directive");
		if (_index.size() != 0)
		{
			std::cout << "WARNING : index already populated earlier, overwriting with current directive" << std::endl;
			_index.clear();
		}
		for (size_t i = 0; i < args.size(); i++)
		{
			std::string	fileName = args[i];
			if (fileName.empty() || fileName.find('/') != std::string::npos || fileName == "." || fileName == "..")
				throw std::runtime_error("Invalid index filename '" + fileName + "'");
			_index.push_back(args[i]);
		}
	}

	void	setAutoIndex(const Directive* directive)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1)
			throw std::runtime_error("Missing arguments in autoindex directive");
		if (_autoindex.isSet == true)
			throw std::runtime_error("Duplicate autoindex directive found");
		if (args[0] == "on")
		{
			_autoindex.isSet = true;
			_autoindex.value = true;
		}
		else if (args[0] == "off")
		{
			_autoindex.isSet = true;
			_autoindex.value = false;
		}
		else
			throw std::runtime_error("Invalid arguments in autoindex directive : only 'on' and 'off' allowed, and we got '" + args[0] + "'");		
	}

	void	checkValidErrorUri(const std::string& uri)
	{
		std::string	http = "http://";
		std::string https = "https://";
		
		if (uri.empty())
			throw std::runtime_error("Empty URI in error_pages directive");
		if (uri[0] == '/' && uri.size() > 1)
		{
			if (containsDoubleDotsAccess(uri))
				throw std::runtime_error("Invalid URI in error_pages directive containing double dots : '" + uri + "'");
			return ;
		}
		if ((!uri.compare(0, http.size(), http) && (uri.size() > http.size()))
			|| (!uri.compare(0, https.size(), https) && (uri.size() > https.size())))
			return ;
		throw std::runtime_error("Invalid URI in error_pages directive : '" + uri + "'");;
	}

	void	setErrorPages(const Directive* directive)
	{
		std::vector<std::string>	args = directive->arguments;
		
		if (args.size() < 2)
			throw std::runtime_error("Missing arguments in error_pages directive");
		
		size_t	last = args.size() - 1;
		checkValidErrorUri(args[last]);
		for (size_t i = 0; i < last; i++)
		{
			u_int	error;
			if (strToVal<u_int>(args[i], error) || error < 400 || error > 599)
				throw std::runtime_error("Invalid error code in error_pages directive");
			if (_errorPages.find(error) != _errorPages.end())
				std::cout << "WARNING : Error " << error << " already mapped in error_pages, overwritten by current directive" << std::endl;
			_errorPages[error] = args[last];
		}
	}

	void	setClientMaxBodySize(const Directive* directive)
	{
		std::vector<std::string>	args = directive->arguments;

		if (args.size() != 1)
			throw std::runtime_error("Missing arguments in client_max_body_size directive");
		if (_clientMaxBodySize.isSet == true)
			throw std::runtime_error("Duplicate client_max_body_size directive");
		size_t	bytes;
		if (strToBytes(args[0], bytes) == false)
			throw std::runtime_error("Invalid value '" + args[0] + "' in client_max_body_size directive");
		_clientMaxBodySize.isSet = true;
		_clientMaxBodySize.value = bytes;
	}

	void	setRoot(const std::string& root) { _root = root; }
	void	setIndex(const std::vector<std::string>& index) { _index = index; }
	void	setAutoindex(const AutoIndexSettings& autoindex) { _autoindex = autoindex; }
	void	setErrorPages(const std::map<int, std::string>& errorPages) { _errorPages = errorPages; }
	void	setClientMaxBodySize(const ClientMaxBodySize& clientMaxBodySize) { _clientMaxBodySize = clientMaxBodySize; }

	const std::string&					root(void) const { return _root; }
	const std::vector<std::string>& 	index(void) const { return _index; }
	const AutoIndexSettings				autoindex(void) const { return _autoindex; }
	const std::map<int, std::string>& 	errorPages(void) const { return _errorPages; }
	const ClientMaxBodySize 			clientMaxBodySize(void) const { return _clientMaxBodySize; }

};



#endif