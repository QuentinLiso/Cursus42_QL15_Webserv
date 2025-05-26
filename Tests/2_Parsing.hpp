/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   2_Parsing.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/26 11:48:20 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	PARSING_HPP
# define PARSING_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"
# include "0A_DataStructures.hpp"
# include "1_Lexing.hpp"



// ==================== PARSER =======================

class	Parser
{
private:
	const std::vector<Token>&	_tokens;
	size_t						_index;
	std::vector<AParsingNode*>	_ast;
	int							_error;

	void			error(int line, int column, const TStr& name, const TStrVect& args, const TStr& msg);
	void			error(const TStr& msg);
	void			parseTokens(void);
	AParsingNode* 	buildNode(void);


public:
	Parser(const std::vector<Token>& tokens);
	virtual ~Parser(void);
	
	const std::vector<AParsingNode*>&	getAst(void) const;
	void	printNodes(void) const;

	int		getError(void) const;
	void	throwInvalid(void) const;

};


// ==================== ABSTRACT NODE =======================

class	AParsingNode
{
	public:
		enum Type
		{
			BLOCK,
			DIRECTIVE,
			UNKNOWN
		};

	private:
		int					_line;
		int 				_column;
		TStr				_name;
		TStrVect 			_args;
		AParsingNode::Type	_nodeType;
		int					_error;

	protected:

	

	public:
		AParsingNode(int line, int column, const TStr& name, const TStrVect& args, AParsingNode::Type nodeType);
		virtual ~AParsingNode(void);

		const TStr& 			getName(void) const;
		const TStrVect&			getArgs(void) const;
		AParsingNode::Type 		getNodeType (void) const;
		int						getError(void) const;
		void					setError(int error);
		int						error(const TStr& msg);
		void					warning(const TStr& msg);

		virtual std::ostream&	print(std::ostream& o, size_t indent) const = 0;
};


// ==================== BLOCKS =======================

class	ParsingBlock : public AParsingNode
{
	protected:
		Blocks::Type									_blockType;
		virtual const std::set<Directives::Type>&		_allowedDirectives(void) const = 0;
		virtual const std::set<Blocks::Type>&			_allowedSubBlocks(void) const = 0;

		std::vector<AParsingNode *> 					_children;

	public:
		ParsingBlock(int line, int column, const TStr& name, const TStrVect& args, Blocks::Type blockType);
		virtual ~ParsingBlock(void);

		Blocks::Type			getBlockType(void) const;
		virtual bool			isAllowedDirective(Directives::Type directiveType);
		virtual bool			isAllowedSubBlock(Blocks::Type blockType);

		void					addChild(AParsingNode* child);
		
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};


class ServerConfigBlock : public ParsingBlock
{
	private:
		virtual const std::set<Directives::Type>&	_allowedDirectives(void) const;
		virtual const std::set<Blocks::Type>&		_allowedSubBlocks(void) const;

	public:
		ServerConfigBlock(int line, int column, const TStr& name, const TStrVect& args);
		virtual ~ServerConfigBlock(void);
};


class LocationConfigBlock : public ParsingBlock
{
	private:
		virtual const std::set<Directives::Type>&	_allowedDirectives(void) const;
		virtual const std::set<Blocks::Type>&		_allowedSubBlocks(void) const;

	public:
		LocationConfigBlock(int line, int column, const TStr& name, const TStrVect& args);
		virtual ~LocationConfigBlock(void);
};










// ==================== DIRECTIVES =======================

class	ParsingDirective : public AParsingNode
{
	private:
		Directives::Type	_directiveType;

	public:
		ParsingDirective(int line, int column, const TStr& name, const TStrVect& args, Directives::Type directiveType);
		virtual ~ParsingDirective(void);

		Directives::Type 		getDirectiveType(void) const;
		virtual std::ostream&	print(std::ostream& o, size_t indent) const;
};


class	Listen : public ParsingDirective
{
	private:
		TStr 	_host;
		ushort 	_port;

		int	setHostAndPort(void)
		{
			const TStrVect& args = getArgs();
			if (args.size() != 1)
				return(error("Invalid number of arguments for listen directive"));

			std::size_t	sep = args[0].find(':');
			if (sep == std::string::npos)
			{
				warning("Server listen directive has no IP address defined, adding '0.0.0.0' by default");
				_host = "0.0.0.0";
				return (setPort(args[0]));
			}
			else
			{
				setIp(args[0].substr(0, sep));
				return (setPort(args[0].substr(sep + 1)));
			}
		}

		int	setPort(const std::string& arg)
		{
			unsigned short	portValue;
			bool			isValid = strToVal<unsigned short>(arg, portValue);
			
			if (!isValid || portValue == 0)
				return (error("Invalid port, port must be a value between 1 and 65535"));
			_port = portValue;
			return (0);
		}
			
		int	setIp(const std::string& ip)
		{
			if (ip == "localhost")
			{
				_host = "127.0.0.1";
				return (0);
			}
				
			TStrVect	fields = split(ip, ".");
			if (fields.size() != 4)
				return (error("Invalid IP address"));
			for (size_t	i = 0; i < 4; i++)
			{
				unsigned char	val;
				if (strToVal<u_char>(fields[i], val) == false)
					return (error("Invalid IP address"));
			}
			_host = ip;
			return (0);
		}


	public:
		Listen(int line, int column, TStr& name, TStrVect& args)
				: ParsingDirective(line, column, name, args, Directives::LISTEN), _host(""), _port(0)
		{}
		~Listen(void) {}

		virtual void	parse(const TStrVect& args)
		{
			setHostAndPort();
		}

		// void	print(std::ostream& o) const
		// {
		// 	o << "Host : " << _host << "\nPort : " << _port << std::endl;
		// }
};

class	ServerName : public ParsingDirective
{
	private:
		TStrVect	_serverNames;
		
		int	setServerNames(void)
		{
			const TStrVect& args = getArgs();
			if (args.size() == 0)
				return (error("Missing arguments in server_name directive"));
			for (size_t i = 0; i < args.size(); i++)
			{
				if (!args[i].empty())
					_serverNames.push_back(args[i]);
			}
			return (0);
		}

	public:
		ServerName(int line, int column, TStr& name, TStrVect& args)
				: ParsingDirective(line, column, name, args, Directives::SERVER_NAME)
		{}
		~ServerName(void) {}

		// void	print(std::ostream& o) const
		// {
		// 	o << "Server names :";
		// 	for (size_t i = 0; i < _serverNames.size(); i++)
		// 		o << " " << _serverNames[i];
		// 	o << std::endl;
		// }
};


class	Alias : public ParsingDirective
{
	private:
		TStr	_folderPath;
		
		int	setFolderPath(void)
		{
			const TStrVect& args = getArgs();
			if (args.size() != 1 || args[0].empty())
				return (error("Alias directive must have exactly 1 non-empty argument"));
			
			const TStr& folderPath = args[0];
			int errorFound = 0;

			if (folderPath[0] != '/')
				errorFound = error("Alias directive must start with a '/'");
			if (!isValidFilepath(folderPath))
				errorFound = error("Invalid folder path character found in alias directive");
			if (containsDoubleDotsAccess(folderPath))
				errorFound = error("Alias directive cannot have '..' as path access inside the provided argument");
			if (!isExecutableDirectory(folderPath))
				errorFound = error("Alias directive folder path is not an accessible directory");
				
			_folderPath = folderPath;
			normalizeFilepath(_folderPath);
			return (errorFound);
		}

	public:
		Alias(int line, int column, TStr& name, TStrVect& args)
				: ParsingDirective(line, column, name, args, Directives::ALIAS)
		{}

		~Alias(void) {}

		// void	print(std::ostream& o) const
		// {
		// 	o << "Alias : " << _folderPath << std::endl;
		// }
};

class	AllowedMethods : public ParsingDirective
{
	private:

		std::set<HttpMethods::Type>	_allowedMethods;

		int	setAllowedMethods(void)
		{
			const TStrVect& args = getArgs();
			if (args.size() == 0)
				return (error("Missing arguments in allowed_methods directive"));
			
			for (size_t i = 0; i < args.size(); i++)
			{
				const TStr& arg = args[i];
				HttpMethods::Type	method;
				if (HttpMethods::map().find(arg, method) == false)
				{
					error("Invalid HTTP method '" + arg + "' found in allowed_methods directive");
					continue ;
				}
				if (_allowedMethods.insert(method).second == false)
					warning("Duplicate HTTP method '" + arg + "' found in allowed_methods directive");
			}
			return (0);
		}


	public:
		AllowedMethods(int line, int column, TStr& name, TStrVect& args)
				: ParsingDirective(line, column, name, args, Directives::ALLOWED_METHODS)
		{}
		~AllowedMethods(void) {}

		// void	print(std::ostream& o) const
		// {
		// 	std::set<HttpMethods::Type>::const_iterator	it = _allowedMethods.begin();
		// 	std::set<HttpMethods::Type>::const_iterator	ite = _allowedMethods.end();

		// 	o << "Allowed HTTP methods:";
		// 	for (; it != ite; it++)
		// 	{
		// 		TStr method;
		// 		HttpMethods::map().find(*it, method);
		// 		o << " " << method;
		// 	}
		// 	o << std::endl;
		// }
};

class	CgiPass : public ParsingDirective
{
	private:
		TStr	_filePath;
		
		int		setFolderPath(void)
		{
			const TStrVect& args = getArgs();
			if (args.size() != 1 || args[0].empty())
				return (error("cgi_pass directive should have exactly 1 non-empty argument"));
			
			int	errorFound = 0;
			const TStr&	filePath = args[0];
			if (filePath[0] != '/')
				errorFound = error("cgi_pass directive argument must be an absolute path and must start with '/'");
			if (checkCgiBin(filePath))
				errorFound = 1;
			if (checkCgiDirectory(filePath))
				errorFound = 1;
			_filePath = filePath;
			return (errorFound);
		}

		int	checkCgiBin(const TStr& filePath)
		{
			struct stat	fileStatus;

			if (stat(filePath.c_str(), &fileStatus) != 0)
				return (error("cgi_pass file does not exist"));
			
			if (!S_ISREG(fileStatus.st_mode))
				return (error("cgi_pass file is not a file"));
				
			if (access(filePath.c_str(), X_OK))
				return (error("cgi_pass file is not executable"));
			return (0);
		}

		int	checkCgiDirectory(const TStr& filePath)
		{
			struct stat dirStatus;

			TStr dir = filePath.substr(0, filePath.find_last_of('/'));	
			if (stat(dir.c_str(), &dirStatus) == 0 && dirStatus.st_mode & S_IWOTH)
				return (error("cgi_pass file directory is world writable, unsafe for cgi_pass"));
			return (0);
		}

	public:
		CgiPass(int line, int column, TStr& name, TStrVect& args)
				: ParsingDirective(line, column, name, args, Directives::CGI_PASS)
		{}
		~CgiPass(void) {}

		// void	print(std::ostream& o) const
		// {
		// 	o << "Cgi Pass: " << _filePath << std::endl;
		// }
};



class	Root : public ParsingDirective
{
	private:
		TStr _folderPath;
	
		int		setFolderPath(void)
		{
			const TStrVect& args = getArgs();
			
			if (args.size() != 1 || args[0].empty())
				return (error("root directive must have exactly 1 non-empty argument"));
			
			int	errorFound = 0;
			const TStr& folderPath = args[0];

			if (folderPath[0] != '/')
				errorFound = error("root directive argument must start with '/'");
			if (!isValidFilepath(folderPath))
				errorFound = error("Invalid folder path character found in root directive");
			if (containsDoubleDotsAccess(folderPath))
				errorFound = error("root directive cannot have '..' as root access inside the provided argument");
			if (!isExecutableDirectory(folderPath))
				errorFound = error("root directive folder path is not an accessible directory");

			_folderPath = folderPath;
			normalizeFilepath(_folderPath);
			return (errorFound);
		}

	public:
		Root(int line, int column, TStr& name, TStrVect& args)
				: ParsingDirective(line, column, name, args, Directives::ROOT)
		{}

		~Root(void) {}


		// void	print(std::ostream& o) const
		// {
		// 	o << "Root: " << _folderPath << std::endl;
		// }
};

class	Index : public ParsingDirective
{
	private:
		TStrVect	_fileNames;
		
		int		setFileNames(void)
		{
			const TStrVect& args = getArgs();

			if (args.size() == 0)
				return (error("Index directive must have at lease 1 argument"));
			
			for (size_t i = 0; i < args.size(); i++)
			{
				const TStr& fileName = args[i];
				if (fileName.empty() || fileName.find('/') != std::string::npos || fileName == "." || fileName == "..")
				{
					warning("Invalid filename '" + fileName + "' in index directive");
					continue ;
				}
				_fileNames.push_back(fileName);
			}
			if (_fileNames.empty())
				warning("No correct filename found in the index directive, index is empty then");
			return (0);
		}

	public:
		Index(int line, int column, TStr& name, TStrVect& args)
				: ParsingDirective(line, column, name, args, Directives::INDEX)
		{}
		~Index(void) {}

		// void	print(std::ostream& o) const
		// {
		// 	o << "Index :";
		// 	for (size_t i = 0; i < _fileNames.size(); i++)
		// 		o << " " << _fileNames[i];
		// 	o << std::endl;
		// }
};

class	Autoindex : public ParsingDirective
{
	private:
		bool	_active;

		int	setAutoIndex(void)
		{
			const TStrVect&	args = getArgs();
			if (args.size() != 1)
				return (error("Autoindex directive must have exactly 1 argument 'on' or 'off'"));
			
			const TStr& arg = args[0];
			if (arg == "on")
			{
				_active = true;
				return (0);
			}
			else if (arg == "off")
			{
				_active = false;
				return (0);
			}
			else
				return (error("Autoindex directive must have exactly 1 argument 'on' or 'off'"));
		}
		
	public:
		Autoindex(int line, int column, TStr& name, TStrVect& args)
				: ParsingDirective(line, column, name, args, Directives::AUTOINDEX)
		{}
		~Autoindex(void) {}

		// void	print(std::ostream& o) const
		// {
		// 	o << "Autoindex : " << (_active ? "on" : "off") << std::endl;
		// }
};

class	ErrorPage : public ParsingDirective
{
	private:
		std::map<ushort, TStr>	_errorPages;

		int	checkValidUri(const TStr& uri)
		{
			if (uri.empty())
				return (error("URI cannot be empty in error_pages directive"));
			if (uri[0] == '/' && uri.size() > 1)
			{
				if (containsDoubleDotsAccess(uri))
					return (error("URI '" + uri + "' cannot contain double dots access in error_pages directive"));
				if (!isExistingAndAccessibleFile(uri, R_OK))
					return (error("URI '" + uri + "' is not an existing read-only accessible file"));
				return (0);
			}
			return(error("Invalid URI '" + uri + "' in error_pages directive"));
		}

	public:
		ErrorPage(int line, int column, TStr& name, TStrVect& args)
				: ParsingDirective(line, column, name, args, Directives::ERROR_PAGE)
		{}
		
		int	addErrorPages(const TStrVect& args)
		{
			if (args.size() < 2)
				return (error("error_pages directive must have at least 2 arguments"));

			size_t	last = args.size() - 1;
			checkValidUri(args[last]);
			int	errorFound = 0;

			for (size_t i = 0; i < last; i++)
			{
				const TStr&	arg = args[i];
				ushort	httpErrnum;
				if (strToVal(arg, httpErrnum) == false || httpErrnum < 400 || httpErrnum > 599)
				{
					errorFound = error("Invalid http error code '" + arg + "' in error_pages directive, http error code must be a value between 400 and 599");
					continue ;
				}		
				if (_errorPages.find(httpErrnum) != _errorPages.end())
					warning("Error code '" + arg + "' already mapped earlier in error_pages directive, overwritten with current directive");
				_errorPages[httpErrnum] = args[last];
			}
			return (errorFound);
		}

		~ErrorPage(void) {}

		// void	print(std::ostream& o) const
		// {
		// 	std::map<ushort, TStr>::const_iterator it = _errorPages.begin();
		// 	std::map<ushort, TStr>::const_iterator ite = _errorPages.end();
			
		// 	o << "Error pages:\n";
		// 	for (; it != ite; it++)
		// 		o << "\t" << it->first << ": " << it->second << std::endl;
		// }
};



class	ClientMaxBodySize : public ParsingDirective
{
	private:
		size_t	_maxBytes;
		
		int	setBytes(void)
		{
			const TStrVect& args = getArgs();
			
			if (args.size() != 1 || args[0].empty())
				return (error("client_max_body_size directive must have exactly 1 non-empty argument"));
			
			const TStr& arg = args[0];
			if (strToBytes(arg, _maxBytes) == false || _maxBytes == 0)
				return (error("Invalid bytes number '" + arg + "' in client_max_body_size directive, bytes must be a number between 1 and 100M"));
			return (0);
		}

	// public:
	// 	ClientMaxBodySize(int line, int column, TStr& name, TStrVect& args)
	// 			: ParsingDirective(line, column, name, args, Directives::CLIENT_MAX_BODY_SIZE)
	// 	{}
	// 	~ClientMaxBodySize(void) {}

	// 	void	print(std::ostream& o) const
	// 	{
	// 		o << "Client max body size bytes: " << _maxBytes << std::endl;
	// 	}
};





#endif