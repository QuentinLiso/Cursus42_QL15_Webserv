/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_Build.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/22 08:45:05 by qliso             #+#    #+#             */
/*   Updated: 2025/06/04 17:56:04 by qliso            ###   ########.fr       */
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
		ElementConfig(const ElementConfig& c);
		ElementConfig& operator=(const ElementConfig& c);
		virtual ~ElementConfig(void);

		int					getLine(void) const;
		int					getColumn(void) const;
		const TStr& 		getName(void) const;
		const TStrVect&		getArgs(void) const;
		int					getError(void) const;
		
		void				updateElementConfig(Statement* statement);
		void				updateElementConfig(int line, int column, const TStr& name, const TStrVect& args);
		void				setError(int error);
		int					error(const TStr& msg);
		void				warning(const TStr& msg);
		// virtual std::ostream&	print(std::ostream& o, size_t indent) const = 0;
};


class	Listen : public ElementConfig
{
	private:
		TStr 		_IP;
		u_int32_t	_IPHostByteOrder;
		u_int16_t 	_portHostByteOrder;

		bool	_set;

		int	setIPAndPort(const TStrVect& args);
		int	setPort(const std::string& arg);
		int	setIp(const std::string& ip);

	public:
		Listen(void);
		Listen(const Listen& c);
		Listen& operator=(const Listen& c);
		~Listen(void);
		
		const TStr&	getIP(void) const;
		u_int32_t	getIPHostByteOrder(void) const;
		u_int16_t	getPortHostByteOrder(void) const;
		bool	isSet(void) const;
		
		TIPPort	toRuntimeConfig(void) const;

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
		ServerName(const ServerName& c);
		ServerName& operator=(const ServerName& c);
		~ServerName(void);

		const std::set<TStr>& getServerNames(void) const;
		bool	hasServerName(const TStr& name) const;
		bool	isSet(void) const;
		
		int	set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};


class	LocationPath : public ElementConfig
{
	private:
		TStr	_path;
		bool	_set;

		int		setPath(const TStrVect& args);
		int		setPath(const TStr& arg);

	public:
		LocationPath(void);
		LocationPath(const LocationPath& c);
		LocationPath& operator=(const LocationPath& c);
		~LocationPath(void);

		const TStr& getPath(void) const;
		bool		isSet(void) const;

		int		set(Statement* statement);
		int		set(const TStr& path);
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
		Alias(const Alias& c);
		Alias& operator=(const Alias&c);
		~Alias(void);

		const TStr& getFolderPath(void) const;
		bool	isSet(void) const;

		int	set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	AllowedMethods : public ElementConfig
{
	private:

		std::set<HttpMethods::Type>	_allowedMethods;
		std::set<TStr>				_allowedMethodsStr;
		bool						_set;

		int	addAllowedMethods(const TStrVect& args);
		
	public:
		AllowedMethods(void);
		AllowedMethods(const AllowedMethods& c);
		AllowedMethods& operator=(const AllowedMethods& c);
		~AllowedMethods(void);

		const std::set<HttpMethods::Type>& getAllowedMethods(void) const;
		bool	isAllowedMethod(HttpMethods::Type method) const;
		bool	isAllowedMethod(const TStr& method) const;
		bool	isSet(void) const;

		void	setDefaultMethods(void);
		int	set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	CgiPass : public ElementConfig
{
	private:
		TStr	_execPath;
		bool	_set;
		
		int	setExecPath(const TStrVect& args);
		int	checkCgiBin(const TStr& filePath);
		int	checkCgiDirectory(const TStr& filePath);

	public:
		CgiPass(void);
		CgiPass(const CgiPass& c);
		CgiPass& operator=(const CgiPass& c);
		~CgiPass(void);

		const TStr& getExecPath(void) const;
		bool		isSet(void) const;

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
		Root(const Root& c);
		Root& operator=(const Root& c);
		~Root(void);

		const TStr& getFolderPath(void) const;
		bool		isSet(void) const;

		int		set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	Index : public ElementConfig
{
	private:
		TStrVect	_fileNames;
		TStrVect	_fullFileNames;
		bool		_set;

		int		setFileNames(const TStrVect& args);

	public:
		Index(void);
		Index(const Index& c);
		Index& operator=(const Index& c);
		~Index(void);

		const TStrVect& getFileNames(void) const;
		const TStrVect& getFullFileNames(void) const;
		bool	isSet(void) const;

		int			set(Statement* statement);
		int			setFullFileNames(const TStr& configFullPath);
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
		Autoindex(const Autoindex& c);
		Autoindex& operator=(const Autoindex& c);
		~Autoindex(void);

		bool	isActive(void) const;
		bool	isSet(void) const;

		int		set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};

class	ErrorPage : public ElementConfig
{
	private:
		std::map<ushort, TStr>	_errorPages;
		std::map<ushort, TStr>	_errorPagesFullPath;

		int	checkValidUri(const TStr& uri);
		int	addErrorPages(const TStrVect& args);


	public:
		ErrorPage(void);
		ErrorPage(const ErrorPage& c);
		ErrorPage& operator=(const ErrorPage& c);
		~ErrorPage(void);

		const std::map<ushort, TStr>& getErrorPages (void) const;
		const TStr* getErrorPage(ushort errnum) const;
		const TStr* getErrorPageFullPath(ushort errnum) const;
		bool isSet(void) const;
		
		void	inheritErrorPages(const ErrorPage& c);
		int		set(Statement* statement);
		int		setFullPath(const TStr& configFullPath);
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
		ClientMaxBodySize(const ClientMaxBodySize& c);
		ClientMaxBodySize& operator=(const ClientMaxBodySize& c);
		~ClientMaxBodySize(void);

		size_t	getMaxBytes(void) const;
		bool	isSet(void) const;

		int	set(Statement* statement);
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};


class ServerConfig;

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
		
		TStr				_fullPath;
		

	public:
		LocationConfig(const ServerConfig& serverConfig);
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
		const TStr&					getFullPath(void) const;


		int	setLocationPath(Statement* statement);
		int	setLocationPath(const TStr& path);
		int	setAlias(Statement* statement);
		int	setAllowedMethods(Statement* statement);
		int	setCgiPass(Statement* statement);
		int	setRoot(Statement* statement);
		int	setIndex(Statement* statement);
		int	setAutoindex(Statement* statement);
		int	setErrorPage(Statement* statement);
		int	setClientMaxBodySize(Statement* statement);
		
		int	setDirective(Statement* statement);

		int	inheritFromServerConfig(const ServerConfig* serverConfig);
		int	setFullPath(void);
		int	validLocationConfig(void);

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
		ClientMaxBodySize _clientMaxBodySize;
		
		std::vector<LocationConfig*>	_locations;
		LocationConfig*					_defaultLocationConfig;
		std::vector<std::pair<TStr, const LocationConfig*> > _runtimeLocations;
		
	public:
		ServerConfig(Statement* statement);
		virtual ~ServerConfig(void);

		const Listen&			getListen		(void) const;
		const ServerName&		getServerName	(void) const;
		const Root&				getRoot			(void) const;
		const Index&			getIndex		(void) const;
		const Autoindex& 		getAutoindex	(void) const;
		const ErrorPage&		getErrorPage	(void) const;
		const ClientMaxBodySize& getClientMaxBodySize(void) const;
		const std::vector<LocationConfig*>	getLocations(void) const;

		int	setListen(Statement* statement);
		int	setServerName(Statement* statement);
		int	setRoot(Statement* statement);
		int	setIndex(Statement* statement);
		int	setAutoindex(Statement* statement);
		int	setErrorPage(Statement* statement);
		int	setClientMaxBodySize(Statement* statement);


		int		setDirective(Statement* statement);
		void	addLocation(LocationConfig* locationConfig);
		void	validateLocations(void);

		int	setDefaultLocationConfig(void);
		const LocationConfig*	getDefaultLocationConfig(void) const;
		int		addDefaultLocationConfig(void);
		int 	checkPort(void);
		int		checkDuplicatePaths(void);
		int		validServerConfig(void);
		
		static bool	compareLocationPairsByPathDescendingLen(const std::pair<TStr, const LocationConfig*>& a, const std::pair<TStr, const LocationConfig*>& b);
		void	makeRuntimeLocations(void);
		const LocationConfig*	findLocation(const TStr& uri) const;
		
		std::ostream&	print(std::ostream& o, size_t indent) const;
};


class	Builder
{
	private:
		std::vector<ServerConfig*>			_build;
		std::map<TIPPort, HostToServerMap>	_runtimeBuild;
		int		_error;

		ServerConfig*	buildServerConfig(Statement* statement);
		LocationConfig*	buildLocationConfig(Statement* child);
		int				checkOverlappingWindlcartPorts(void);

		void	setError(int error);
		int		error(Statement* statement, const TStr& msg);
		int		error(const ElementConfig* elementConfig, const TStr& msg);
		int		error(const TStr& msg);
	
	public:
		Builder(void);
		~Builder(void);

		int		getError(void) const;
		const std::vector<ServerConfig*>&		  getRawBuild(void) const;
		const std::map<TIPPort, HostToServerMap>& getRuntimeBuild(void) const;

		void				parsingToBuild(const std::vector<Statement*>& statements);
		int					validParsingToBuild(void);
		void				makeRuntimeBuild(void);
		int					validRuntimeBuild(void);
		
		// std::set<TIPPort>		getBoundSockets(void) const;
		// const HostToServerMap&	getServerConfigsForSocket(const TIPPort &ipPort);
		// const ServerConfig*		findServerConfig(const TIPPort& ipPort, const TStr& host) const;
		// const LocationConfig*	findLocationConfig(const TIPPort& ipPort, const TStr& host, const TStr& requestedUri) const;

		void	printBuild(void) const;
		void	printRuntimeBuild(void) const;
		void	throwInvalid(const TStr& msg) const;
};


#endif