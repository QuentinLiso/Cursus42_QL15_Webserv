/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_Build.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/22 08:45:05 by qliso             #+#    #+#             */
/*   Updated: 2025/05/27 19:14:11 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BUILD_HPP
# define BUILD_HPP

# include "Includes.hpp"
# include "0_Utils.hpp"
# include "1_Lexing.hpp"
# include "2_Parsing.hpp"


class	ElementConfig
{
	protected:
		int			_line;
		int 		_column;
		TStr		_name;
		TStrVect 	_args;
		int			_error;

	public:
		ElementConfig(void);
		ElementConfig(Statement* statement);
		ElementConfig(int line, int column, const TStr& name, const TStrVect& args);
		virtual ~ElementConfig(void);

		int					getLine(void) const;
		int					getColumn(void) const;
		const TStr& 		getName(void) const;
		const TStrVect&		getArgs(void) const;
		int					getError(void) const;
		
		void				updateElementConfig(Statement* statement);
		void				setError(int error);
		int					error(const TStr& msg);
		void				warning(const TStr& msg);
		// virtual std::ostream&	print(std::ostream& o, size_t indent) const = 0;
};


class	Listen : public ElementConfig
{
	private:
		TStr 	_host;
		ushort 	_port;
		bool	_set;

		int	setHostAndPort(const TStrVect& args);
		int	setPort(const std::string& arg);
		int	setIp(const std::string& ip);

	public:
		Listen(void);
		~Listen(void);
		
		int			set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	ServerName : public ElementConfig
{
	private:
		std::set<TStr>	_serverNames;
		bool			_set;

		int	addServerNames(const TStrVect& args);

	public:
		ServerName(void);
		~ServerName(void);

		int	set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};


class	LocationPath : public ElementConfig
{
	private:
		TStr	_path;
		bool	_set;

		int		setPath(const TStrVect& args);

	public:
		LocationPath(void);
		~LocationPath(void);

		int		set(Statement* statement);
		virtual std::ostream& print(std::ostream& o, size_t indent) const;
};

class	Alias : public ElementConfig
{
	private:
		TStr	_folderPath;
		bool	_set;
		
		int	setFolderPath(const TStrVect& args);

	public:
		Alias(void);
		~Alias(void);

		int	set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	AllowedMethods : public ElementConfig
{
	private:

		std::set<HttpMethods::Type>	_allowedMethods;
		bool						_set;

		int	addAllowedMethods(const TStrVect& args);
		
	public:
		AllowedMethods(void);
		~AllowedMethods(void);

		int	set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	CgiPass : public ElementConfig
{
	private:
		TStr	_filePath;
		bool	_set;
		
		int	setFolderPath(const TStrVect& args);
		int	checkCgiBin(const TStr& filePath);
		int	checkCgiDirectory(const TStr& filePath);

	public:
		CgiPass(void);
		~CgiPass(void);

		int		set(Statement* statement);
		
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};



class	Root : public ElementConfig
{
	private:
		TStr 	_folderPath;
		bool	_set;
	
		int		setFolderPath(const TStrVect& args);

	public:
		Root(void);
		~Root(void);

		int		set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	Index : public ElementConfig
{
	private:
		TStrVect	_fileNames;
		bool		_set;

		int		setFileNames(const TStrVect& args);

	public:
		Index(void);
		~Index(void);

		int		set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	Autoindex : public ElementConfig
{
	private:
		bool	_active;
		bool	_set;

		int	setAutoIndex(const TStrVect& args);
		
	public:
		Autoindex(void);
		~Autoindex(void);

		int		set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	ErrorPage : public ElementConfig
{
	private:
		std::map<ushort, TStr>	_errorPages;

		int	checkValidUri(const TStr& uri);
		int	addErrorPages(const TStrVect& args);

	public:
		ErrorPage(void);
		~ErrorPage(void);
		
		int	set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	ClientMaxBodySize : public ElementConfig
{
	private:
		size_t	_maxBytes;
		bool	_set;
		
		int	setBytes(const TStrVect& args);

	public:
		ClientMaxBodySize(void);
		~ClientMaxBodySize(void);

		int	set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};



class	LocationConfig : public ElementConfig
{
	private:
		LocationPath		_locationPath;
		Alias				_alias;
		AllowedMethods		_allowedMethods;
		CgiPass				_cgiPass;
		Root				_root;
		Index				_index;
		Autoindex			_autoindex;
		ErrorPage			_errorPage;
		ClientMaxBodySize	_clientMaxBodySize;

	public:
		LocationConfig(Statement* statement);
		virtual ~LocationConfig(void);

		const LocationPath&			getLocationPath(void) const;
		const Alias&				getAlias(void) const;
		const AllowedMethods&		getAllowedMethods(void) const;
		const CgiPass&				getCgiPass(void) const;
		const Root&					getRoot(void) const;
		const Index&				getIndex(void) const;
		const Autoindex&			getAutoindex(void) const;
		const ErrorPage&			getErrorPage(void) const;
		const ClientMaxBodySize&	getClientMaxBodySize(void) const;

		int	setLocationPath(Statement* statement);
		int	setAlias(Statement* statement);
		int	setAllowedMethods(Statement* statement);
		int	setCgiPass(Statement* statement);
		int	setRoot(Statement* statement);
		int	setIndex(Statement* statement);
		int	setAutoindex(Statement* statement);
		int	setErrorPage(Statement* statement);
		int	setClientMaxBodySize(Statement* statement);
		
		int	setDirective(Statement* statement);

		std::ostream&	print(std::ostream& o, size_t indent) const;
};


class	ServerConfig : public ElementConfig
{
	private:
		Listen		_listen;
		ServerName	_serverName;
		Root		_root;
		Index		_index;
		Autoindex	_autoindex;
		ErrorPage	_errorPage;
		std::vector<LocationConfig*>	_locations;
		
	public:
		ServerConfig(Statement* statement);
		virtual ~ServerConfig(void);

		const Listen&			getListen		(void) const;
		const ServerName&		getServerName	(void) const;
		const Root&				getRoot			(void) const;
		const Index&			getIndex		(void) const;
		const Autoindex& 		getAutoindex	(void) const;
		const ErrorPage&		getErrorPage	(void) const;
		const std::vector<LocationConfig*>	getLocations(void) const;

		int	setListen(Statement* statement);
		int	setServerName(Statement* statement);
		int	setRoot(Statement* statement);
		int	setIndex(Statement* statement);
		int	setAutoindex(Statement* statement);
		int	setErrorPage(Statement* statement);

		int	setDirective(Statement* statement);
		void	addLocation(LocationConfig* locationConfig);

		std::ostream&	print(std::ostream& o, size_t indent) const;
};


class	Builder
{
	private:
		const std::vector<Statement*>& 	_statements;
		std::vector<ServerConfig*>		_build;
		int		_error;

		void			parsingToBuild(void);
		ServerConfig*	buildServerConfig(Statement* statement);
		LocationConfig*	buildLocationConfig(Statement* child);

		int					getError(void) const;
		void				setError(int error);
		int					error(Statement* statement, const TStr& msg);

	public:
		Builder(const std::vector<Statement*>& statements);
		~Builder(void);



		void	printBuild(void) const;
		void	throwInvalid(void) const;
};




#endif