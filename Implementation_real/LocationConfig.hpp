/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   LocationConfig.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/17 23:33:26 by qliso            ###   ########.fr       */
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
	

	void	parseLocationDirective(const Directive* directive)
	{
		if(directive->name == "root")
			_common.setRoot(directive, _alias);
		else if (directive->name == "index")
			_common.setIndex(directive);
		else if (directive->name == "autoindex")
			_common.setAutoIndex(directive);
		else if (directive->name == "error_page")
			_common.setErrorPages(directive);
		else if (directive->name == "client_max_body_size")
			_common.setClientMaxBodySize(directive);
		else if (directive->name == "alias")
			setAlias(directive);
		else if (directive->name == "allowed_methods")
			setAllowedMethods(directive);
		else if (directive->name == "cgi_pass")
			setCgiPass(directive);

		// std::cout << "Directive " + directive->name + " added to the location config" << std::endl;
	}

	void	setLocationPath(const Block* block)
	{
		std::vector<std::string>	args = block->arguments;
		
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid path argument in location statement");
		
		std::string	path = args[0];
		if (path[0] != '/')
			throw std::runtime_error("Location path argument '" + path + "' must start with a '/'");

		if (!isValidFilepath(path))
			throw std::runtime_error("Invalid character found in location path '" + path + "'");

		removeStrDuplicateChar(path, '/');
		if (containsDoubleDotsAccess(path))
			throw std::runtime_error("Filepath " + path + " contains forbidden '..' access");
		_path = path;
	}

	void	setAlias(const Directive* directive)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid argument in root directive");
		if (!_alias.empty())
			throw std::runtime_error("Alias directive aborted: Alias directive already defined in the location, cannot have 2 alias in the same location");
		if (!_common.root().empty())
			throw std::runtime_error("Cannot have both an alias and a root directive inside a location block");
		
		std::string	alias = args[0];
		if (alias[0] != '/')
			throw std::runtime_error("alias directive '" + alias + "' must start with a '/'");
		if (!isValidFilepath(alias))
			throw std::runtime_error("Invalid character found in root directive '" + alias + "'");
		if (containsDoubleDotsAccess(alias))
			throw std::runtime_error("Alias cannot have '..' as path access inside the provided argument");
		isExecutableDirectory(alias);
		_alias = alias;
	}

	
	void	addAllowedMethod(std::set<HttpMethods>& configHttpMethods, HttpMethods methodId, const std::string& methodStr)
	{
		if (configHttpMethods.insert(methodId).second == false)
			std::cout << "WARNING: Duplicate HTTP method '" << methodStr << "' found in allowed_methods directive" << std::endl;
	}

	void	setAllowedMethods(const Directive* directive)
	{
		std::vector<std::string>	args = directive->arguments;
	
		if (args.size() == 0)
			throw std::runtime_error("Missing arguments in allowed_methods directive");
		if (!_allowedMethods.empty())
			throw std::runtime_error("Duplicate allowed_methods directive found in location config '" + _path + "'");
		for (size_t i = 0; i < args.size(); i++)
		{
			std::string&	method = args[i];
			if (method == "GET")
				addAllowedMethod(_allowedMethods, HTTP_GET, method);
			else if (method == "PUT")
				addAllowedMethod(_allowedMethods, HTTP_PUT, method);
			else if (method == "DELETE")
				addAllowedMethod(_allowedMethods, HTTP_DELETE, method);
			else if (method == "POST")
				addAllowedMethod(_allowedMethods, HTTP_POST, method);
			else
				throw std::runtime_error("Invalid argument '" + method + "' in allowed_methods directive");
		}
	}

	void	fillEmptyAllowedMethods(void)
	{
		if (_allowedMethods.empty())
			{
				std::cout << "WARNING: location '" + _path + "' has no allowed_methods directive, adding GET and POST by default" << std::endl;
				_allowedMethods.insert(HTTP_GET);
				_allowedMethods.insert(HTTP_POST);
			}
	}

	
	void	checkCgiBin(const std::string& filePath) const
	{
		struct stat	fileStatus;
		if (stat(filePath.c_str(), &fileStatus) != 0)
			throw std::runtime_error("cgi_pass file '" + filePath + "' does not exist");
		
		if (!S_ISREG(fileStatus.st_mode))
			throw std::runtime_error("cgi_pass file '" + filePath + "' is not a file");
		
		if (access(filePath.c_str(), X_OK))
			throw std::runtime_error("cgi_pass file '" + filePath + "' is not executable");	
	}

	void	checkCgiDirectory(const std::string& filePath) const
	{
		struct stat dirStatus;
		std::string dir = filePath.substr(0, filePath.find_last_of('/'));
		
		if (stat(dir.c_str(), &dirStatus) == 0 && dirStatus.st_mode & S_IWOTH)
			throw std::runtime_error("cgi_pass file directory '" + dir + "' is world writable: Unsafe for cgi_pass");
	}

	void	setCgiPass(const Directive* directive)
	{
		std::vector<std::string>	args = directive->arguments;

		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("cgi_pass directive should have exactly 1 non-empty argument");
		if (!_cgiPass.empty())
			throw std::runtime_error("Duplicate cgi_pass directive encountered");
		if (args[0][0] != '/')
			throw std::runtime_error("cgi_pass '" + args[0] + "'must be an absolute path, so must start with '/'");
		const std::string& cgiPass = args[0];
		checkCgiBin(cgiPass);
		checkCgiDirectory(cgiPass);
		_cgiPass = args[0];
	}

	
	void	inheritCommonFromServerConfig(const ServerConfig& serverConfig)
	{
		_common.inherit(serverConfig.common());
		if (!_alias.empty())
		{
			_common.setRoot("");
			return ;
		}
		else if (_common.root().empty())
			throw std::runtime_error("Empty root at server level is not covered by root or alias at location level '" + _path + "'");
		else
			isExecutableDirectory(joinPaths(_common.root(), _path));			
	}

	
public:
	LocationConfig(const ServerConfig& serverConfig) : _pathModifier(""), _path("/"), _alias(""), _cgiPass("")
	{
		inheritCommonFromServerConfig(serverConfig);
		fillEmptyAllowedMethods();
	}

	LocationConfig(const Block* block, const ServerConfig& serverConfig)
	{
		parseLocationBlock(block, serverConfig);
		inheritCommonFromServerConfig(serverConfig);
		fillEmptyAllowedMethods();
	}
	
	~LocationConfig(void) {}

	const CommonConfig&		common(void) const { return _common; }
	const std::string&		pathModifier(void) const { return _pathModifier; }
	const std::string&		path(void) const { return _path; }
	const std::string&		alias(void) const { return _alias; }
	const std::set<HttpMethods>	allowedMethods(void) const { return _allowedMethods; }
	const std::string&		cgiPass(void) const { return _cgiPass; }

	void	parseLocationBlock(const Block* block, const ServerConfig& serverConfig)
	{
		setLocationPath(block);
		for (size_t i = 0; i < block->children.size(); i++)
		{
			AConfigNode*	child = block->children[i];
			if (child->isBlock())
				throw std::runtime_error("Block found in location block");
			
			Directive*	childDirective = dynamic_cast<Directive*>(child);
			if (!childDirective)
				throw std::runtime_error("Dynamic cast childDirective failed");
			parseLocationDirective(childDirective);
		}
	}

	void	printConfig(size_t i = 0) const
	{
		std::cout 	<< "\t****** LOCATION BLOCK " << i << " ******\n" 
					<< "\tPath modifier : " + _pathModifier + "\n"
					<< "\tPath : " + _path + "\n"
					<< "\tRoot : " + _common.root() + "\n"
					<< "\tAlias : " + _alias + "\n"
					<< "\tIndex : ";
		for (size_t j = 0; j < _common.index().size(); i++)
			std::cout << _common.index()[j] << " ";
		std::cout 	<< "\n"
				 	<< "\tAutoindex : " << ((_common.autoindex().value == true) ? "on" : "off") << "\n"
					<< "\tError pages :\n";
		for (std::map<int, std::string>::const_iterator it = _common.errorPages().begin(); it != _common.errorPages().end(); it++)
			std::cout << "\t\t" << it->first << " : " << it->second << "\n";
		std::cout 	<< "\tClient max body size (in bytes) : " << _common.clientMaxBodySize().value << "\n"
					<< "\tAllowed methods (1: GET, 2: POST, 3: DELETE, 4: PUT) : ";
		for (std::set<HttpMethods>::const_iterator it = _allowedMethods.begin(); it != _allowedMethods.end(); it++)
			std::cout << (*it + 1) << " ";
		std::cout	<< "\n"
					<< "\tCGI Pass : " + _cgiPass
					<< "\n" << std::endl;
	}

};



#endif