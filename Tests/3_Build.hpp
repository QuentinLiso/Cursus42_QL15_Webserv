/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_Build.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/22 08:45:05 by qliso             #+#    #+#             */
/*   Updated: 2025/05/30 01:02:05 by qliso            ###   ########.fr       */
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
		TStr	_filePath;
		TStr	_fullFilePath;
		bool	_set;
		
		int	setFolderPath(const TStrVect& args);
		int	checkCgiBin(const TStr& filePath);
		int	checkCgiDirectory(const TStr& filePath);

	public:
		CgiPass(void);
		CgiPass(const CgiPass& c);
		CgiPass& operator=(const CgiPass& c);
		~CgiPass(void);

		const TStr& getFilePath(void) const;
		const TStr& getFullFilePath(void) const;
		bool		isSet(void) const;

		int		set(Statement* statement);
		int		setFullPath(const TStr& configFullPath);
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
		const std::vector<Statement*>& 	_statements;
		std::vector<ServerConfig*>		_build;
		std::map<TIPPort, HostToServerMap>	_runtimeBuild;

		int		_error;

		void			parsingToBuild(void);
		ServerConfig*	buildServerConfig(Statement* statement);
		LocationConfig*	buildLocationConfig(Statement* child);
		


		
		
		// const std::map<TStr, const ServerConfig*>&	findServerConfigs(TIPPort ipPort);
		// std::map<TStr, const ServerConfig*> createMapServerNamesToServecConfig(u_int32_t ip, u_int16_t port);
	
		int					getError(void) const;
		void				setError(int error);
		int					error(Statement* statement, const TStr& msg);
		int					error(const ElementConfig* elementConfig, const TStr& msg);
	
	public:
		Builder(const std::vector<Statement*>& statements);
		~Builder(void);

		const std::map<TIPPort, HostToServerMap>& getRuntimeBuild(void) const;
		void				makeRuntimeBuild(void);
		int					validRuntimeBuild(void);			//TO IMPLEMENT
		int					checkWildcardIpDuplicatePorts(void); // TO IMPLEMENT
		int				checkFallbacksServers(void);   		// TO IMPLEMENT
				
		const ServerConfig*	findServerConfig(const TIPPort& ipPort, const TStr& host);
		const LocationConfig*	findLocationConfig(const TIPPort& ipPort, const TStr& host, const TStr& requestedUri); // TO IMPLEMENT

		void	printBuild(void) const;
		void	printRuntimeBuild(void) const;	// TO IMPLEMENT
		void	throwInvalid(void) const;
};

// TO IMPLEMENT :::::

// int validRuntimeBuild(void);
// Purpose: Global consistency/structure check of the entire runtime build.

// What to validate:

// No empty _runtimeBuild or empty maps per IP:Port.

// Each (IP, Port) has at least one valid ServerConfig*.

// Each HostToServerMap has a "" (fallback/default).

// No NULL pointers.

// ✅ Good idea. This ensures the runtime map is fully built and sound.

// int checkWildcardIpDuplicatePorts(void);
// Purpose: Avoid conflicts between 0.0.0.0:port and other specific IPs using the same port.

// Why it's important:
// Binding to 0.0.0.0:80 reserves port 80 on all interfaces, so trying to bind 192.168.1.10:80 will fail.

// What to check:

// If 0.0.0.0:PORT exists, no other (IP, PORT) pairs should exist for that PORT.

// This check must cover all combinations of TIPPort.

// ✅ Very good, and often overlooked.

// int checkFallbacksServers(void);
// Purpose: Ensure every (IP, Port) has a default fallback ("") server_name.

// Why it matters:

// The Host: header can be missing, malformed, or unmatched.

// Nginx-like behavior: fallback to the default server {} block for that IP/Port.

// What to check:

// Every HostToServerMap in _runtimeBuild[ipPort] contains a "" entry.

// Maybe warn if the fallback server has server_name entries — it may not match any.

// ✅ Must-have. Avoids crashes or undefined behavior when Host: doesn’t match.

// 🧨 Extra-Picky Suggested Checks
// Now let’s push it further:

// 🔍 1. Detect Same server_name on Conflicting IP:Port
// Why? While server_name can repeat across IP:Port scopes, it could lead to misinterpretation if the configs are incorrect or overlapping.

// What to do:

// Build a reverse index: std::map<TStr, std::set<TIPPort>> serverNameToBindings

// Detect weird overlaps — same server_name defined for wildly different bindings (e.g., localhost on 10.0.0.1:8080 and 192.168.1.1:443)

// 🔸 Optional, but great for user clarity/debugging.

// 🔍 2. Validate Unique listen IP:Port Across ServerConfig Blocks
// Nginx allows you to reuse ports with different server_names — but you can’t have exact same listen IP+port+server_name.

// You already handle duplicates per IP:port, but check if two configs define exactly the same (IP, port, and server_name).

// cpp
// Copy
// Edit
// std::set<std::tuple<TIPPort, TStr>> seenBindings;
// 🔍 3. Check That All ServerConfigs Are Used in _runtimeBuild
// Unused server blocks should be flagged.

// Example:

// A config block with a listen to a wrong IP (typo) that’s never resolved.

// How:

// Track all ServerConfig* in _build

// After makeRuntimeBuild(), check that each one appears at least once in a map.

// 🔍 4. Detect Overlapping Wildcard Port Bindings (across different ports)
// If you're being truly paranoid:

// Two wildcard entries 0.0.0.0:80 and 0.0.0.0:443 are fine

// But make sure wildcard + overlap with specific IP doesn’t cover same logical interface.

// May be out-of-scope for your setup — depends on runtime binding semantics.

// 🔍 5. Warn If Multiple IPPorts Share Same ServerConfig Pointer
// This might be fine (same config reused) — but it’s good to log it explicitly.

// cpp
// Copy
// Edit
// std::map<const ServerConfig*, std::vector<TIPPort>> reverseMap;
// It helps for debug or misconfig detection.

// 🧠 Summary: Final List of Validation Checks
// Check #	Description
// ✅ 1	Ensure each IP:Port has at least one ServerConfig
// ✅ 2	Ensure fallback server ("") exists per HostToServerMap
// ✅ 3	Prevent 0.0.0.0:PORT conflicting with IP:PORT
// 🔍 4	Warn on duplicated (IP, PORT, server_name) triple
// 🔍 5	Detect server_name mapped to multiple IP:PORT (maybe warn)
// 🔍 6	Validate all ServerConfig* are used in _runtimeBuild
// 🔍 7	Optionally warn on reused ServerConfig* across bindings
// 🔍 8	Normalize and compare server_names case-insensitively


#endif