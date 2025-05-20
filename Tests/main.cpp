# include "Includes.hpp"
# include "Console.hpp"
# include "DataStructures.hpp"
# include "Utils.hpp"
# include "Lexer.hpp"

typedef std::vector<std::string>	TStrVect;
typedef std::vector<int>			TIntVect;
typedef std::vector<ushort>			TUShortVect;
typedef std::string					TStr;


template < typename T>
class	BiMap
{
	private:
		std::map<TStr, T>	_strToTypeMap;
		std::map<T, TStr>	_typeToStrMap;


	public:
		BiMap(void) {}
		~BiMap(void) {}

		BiMap&	add(const TStr& str, const T& type)
		{
			_strToTypeMap[str] = type;
			_typeToStrMap[type] = str;
			return (*this);
		}

		bool	find(const TStr& str) const
		{
			typename std::map<TStr, T>::const_iterator it = _strToTypeMap.find(str);
			if (it != _strToTypeMap.end())
				return (true);
			return (false);
		}
		
		bool	find(const TStr& str, T& _type) const
		{
			typename std::map<TStr, T>::const_iterator it = _strToTypeMap.find(str);
			if (it != _strToTypeMap.end())
			{
				_type = it->second;
				return (true);
			}
			return (false);
		}

		bool	find(const T& type) const
		{
			typename std::map<T, TStr>::const_iterator it = _typeToStrMap.find(type);
			if (it != _typeToStrMap.end())
				return (true);
			return (false);
		}

		bool	find(const T& type, TStr& _str) const
		{
			typename std::map<T, TStr>::const_iterator it = _typeToStrMap.find(type);
			if (it != _typeToStrMap.end())
			{
				_str = it->second;
				return (true);
			}
			return (false);
		}

		size_t	size(void) const { return (_strToTypeMap.size()); }
		bool	empty(void) const { return (_strToTypeMap.empty()); }
};

class	HttpMethods
{
	public:
		enum Type
		{
			UNKNOWN,
			GET,
			POST,
			DELETE,
			PUT
		};

	private:
		static void fillBiMap(BiMap<HttpMethods::Type>& bimap)
		{
				bimap.add("GET", HttpMethods::GET);
				bimap.add("POST", HttpMethods::POST);
				bimap.add("DELETE", HttpMethods::DELETE);
				bimap.add("PUT", HttpMethods::PUT);
		}

	public:
		static const BiMap<HttpMethods::Type>&	map(void)
		{ 
			static BiMap<HttpMethods::Type> bimap;
			if (bimap.empty())
				fillBiMap(bimap);
			return (bimap);
		}
};

class	Directives
{
	public:
		enum Type
		{
			UNKNOWN,
			LISTEN,				// Server specific
			SERVER_NAME,		// Server specific

			ALIAS,				// Location specific
			ALLOWED_METHODS,	// Location specific
			CGI_PASS,			// Location specific

			ROOT,
			INDEX,
			AUTOINDEX,
			ERROR_PAGE,
			CLIENT_MAX_BODY_SIZE,
		};

	private:
		static void fillBiMap(BiMap<Directives::Type>& bimap)
		{
				bimap.add("listen", Directives::LISTEN);
				bimap.add("server_name", Directives::SERVER_NAME);

				bimap.add("alias", Directives::ALIAS);
				bimap.add("allowed_methods", Directives::ALLOWED_METHODS);
				bimap.add("cgi_pass", Directives::CGI_PASS);

				bimap.add("root", Directives::ROOT);
				bimap.add("index", Directives::INDEX);
				bimap.add("autoindex", Directives::AUTOINDEX);
				bimap.add("error_page", Directives::ERROR_PAGE);
				bimap.add("client_max_body_size", Directives::CLIENT_MAX_BODY_SIZE);
		}

	public:
		static const BiMap<Directives::Type>&	map(void)
		{ 
			static BiMap<Directives::Type> bimap;
			if (bimap.empty())
				fillBiMap(bimap);
			return (bimap);
		}
};

class	Blocks
{
	public:
		enum Type
		{
			UNKNOWN,
			SERVER,
			LOCATION
		};

	private:
		static void fillBiMap(BiMap<Blocks::Type>& bimap)
		{
				bimap.add("server", Blocks::SERVER);
				bimap.add("location", Blocks::LOCATION);
		}

	public:
		static const BiMap<Blocks::Type>&	map(void)
		{ 
			static BiMap<Blocks::Type> bimap;
			if (bimap.empty())
				fillBiMap(bimap);
			return (bimap);
		}
};




// ==================== ABSTRACT NODE =======================

class	AConfigNode
{
	private:
		int			_line;
		int 		_column;
		TStrVect 	_args;
		TStr		_name;
		int			_error;

	protected:
		const TStrVect&	getArgs(void) const				{ return _args; }
		void			setArgs(const TStrVect& args)	{ _args = args; }
		int				getError(void) const 			{ return _error; };
		void			setError(int error) 			{ _error = error; };

		int				error(const TStr& msg)
		{
			Console::configLog(Console::ERROR, _line, _column, _args, "PARSING", msg);
			setError(1);
			return (1);
		}

		void			warning(const TStr& msg)
		{
			Console::configLog(Console::WARNING, _line, _column, _args, "PARSING", msg);
		}

	
		virtual bool	isBlock(void) const = 0;
		

	public:
		AConfigNode(int line, int column, const TStrVect& args)
				: _line(line), _column(column), _args(args), _name("NODE"), _error(0)
		{}
		virtual ~AConfigNode(void) {}

		virtual void	print(std::ostream& o) const = 0;
};




// ==================== DIRECTIVES =======================

class	ADirective : public AConfigNode
{
	protected:
		Directives::Type	_type;
		virtual bool	isBlock(void) const { return false; }

	public:
		ADirective(int line, int column, const TStrVect& args, Directives::Type type) 
				: AConfigNode(line, column, args), _type(type)
		{}
		virtual ~ADirective(void) {}

		Directives::Type type(void) const { return _type; }
};

// ================

// class	DIRECTIVE : public ADirective
// {
// 	private:
// 		TYPE	_field;	
// 	public:
// 		DIRECTIVE(int line, int column, TStrVect& args)
// 				: ADirective(line, column, args, DIRTYPE)
// 		{
// 		}
// 		~DIRECTIVE(void) {}
// 		void	print(std::ostream& o) const
// 		{
// 			o << " directive";
// 		}
// };

class	Listen : public ADirective
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
		Listen(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::LISTEN), _host(""), _port(0)
		{
			setHostAndPort();
		}
		~Listen(void) {}

		void	print(std::ostream& o) const
		{
			o << "Host : " << _host << "\nPort : " << _port << std::endl;
		}
};

class	ServerName : public ADirective
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
		ServerName(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::SERVER_NAME)
		{
			setServerNames();
		}
		~ServerName(void) {}

		void	print(std::ostream& o) const
		{
			o << "Server names :";
			for (size_t i = 0; i < _serverNames.size(); i++)
				o << " " << _serverNames[i];
			o << std::endl;
		}
};



class	Alias : public ADirective
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
		Alias(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::ALIAS)
		{
			setFolderPath();
		}

		~Alias(void) {}

		void	print(std::ostream& o) const
		{
			o << "Alias : " << _folderPath << std::endl;
		}
};

class	AllowedMethods : public ADirective
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
		AllowedMethods(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::ALLOWED_METHODS)
		{
			setAllowedMethods();
		}
		~AllowedMethods(void) {}

		void	print(std::ostream& o) const
		{
			std::set<HttpMethods::Type>::const_iterator	it = _allowedMethods.begin();
			std::set<HttpMethods::Type>::const_iterator	ite = _allowedMethods.end();

			o << "Allowed HTTP methods:";
			for (; it != ite; it++)
			{
				TStr method;
				HttpMethods::map().find(*it, method);
				o << " " << method;
			}
			o << std::endl;
		}
};

class	CgiPass : public ADirective
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
		CgiPass(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::CGI_PASS)
		{
			setFolderPath();
		}
		~CgiPass(void) {}

		void	print(std::ostream& o) const
		{
			o << "Cgi Pass: " << _filePath << std::endl;
		}
};



class	Root : public ADirective
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
		Root(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::ROOT)
		{
			setFolderPath();
		}

		~Root(void) {}


		void	print(std::ostream& o) const
		{
			o << "Root: " << _folderPath << std::endl;
		}
};

class	Index : public ADirective
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
		Index(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::INDEX)
		{
			setFileNames();
		}
		~Index(void) {}

		void	print(std::ostream& o) const
		{
			o << "Index :";
			for (size_t i = 0; i < _fileNames.size(); i++)
				o << " " << _fileNames[i];
			o << std::endl;
		}
};

class	Autoindex : public ADirective
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
		Autoindex(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::AUTOINDEX)
		{
			setAutoIndex();
		}
		~Autoindex(void) {}

		void	print(std::ostream& o) const
		{
			o << "Autoindex : " << (_active ? "on" : "off") << std::endl;
		}
};

class	ErrorPage : public ADirective
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
		ErrorPage(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::ERROR_PAGE)
		{
			addErrorPages(args);
		}
		
		int	addErrorPages(const TStrVect& args)
		{
			setArgs(args);
			if (args.size() < 2)
				return (error("error_pages directive must have at lease 2 arguments"));

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

		void	print(std::ostream& o) const
		{
			std::map<ushort, TStr>::const_iterator it = _errorPages.begin();
			std::map<ushort, TStr>::const_iterator ite = _errorPages.end();
			
			o << "Error pages:\n";
			for (; it != ite; it++)
				o << "\t" << it->first << ": " << it->second << std::endl;
		}
};

class	ClientMaxBodySize : public ADirective
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

	public:
		ClientMaxBodySize(int line, int column, TStrVect& args)
				: ADirective(line, column, args, Directives::CLIENT_MAX_BODY_SIZE)
		{
			setBytes();
		}
		~ClientMaxBodySize(void) {}

		void	print(std::ostream& o) const
		{
			o << "Client max body size bytes: " << _maxBytes << std::endl;
		}
};



// ==================== BLOCKS =======================

template < typename Derived >
class	ABlock : public AConfigNode
{
	private:
		Blocks::Type								_type;
		std::map<Directives::Type, ADirective* >	_directives;
		std::vector<ABlock *> 						_blocks;

	protected:
		virtual bool	isBlock(void) const { return true; }
		const std::set<Directives::Type>& allowedDirectives(void) const
		{
			return Derived::getAllowedDirectives();
		}	


	public:
		ABlock(int line, int column, const TStrVect& args, Blocks::Type type) 
				: AConfigNode(line, column, args), _type(type)
		{}

		virtual ~ABlock(void) {}
		virtual void parse(void) = 0;
};

// =================

class	ServerBlock : public ABlock<ServerBlock>
{
	private:

		static void	fillAllowedDirectives(std::set<Directives::Type>& allowedDirectives)
		{
			allowedDirectives.insert(Directives::LISTEN);
			allowedDirectives.insert(Directives::SERVER_NAME);

			allowedDirectives.insert(Directives::INDEX);
			allowedDirectives.insert(Directives::ROOT);
			allowedDirectives.insert(Directives::INDEX);
			allowedDirectives.insert(Directives::AUTOINDEX);
			allowedDirectives.insert(Directives::ERROR_PAGE);
			allowedDirectives.insert(Directives::CLIENT_MAX_BODY_SIZE);
		}

	public:
		ServerBlock(int line, int column, const TStrVect& args) 
				: ABlock(line, column, args, Blocks::SERVER)
		{}
		~ServerBlock(void) {}

		virtual void parse(void) {}
		virtual void print(std::ostream& o) const {}

		static const std::set<Directives::Type>& getAllowedDirectives()
		{
			static std::set<Directives::Type>	allowedDirectives;
			if (!allowedDirectives.empty())
				fillAllowedDirectives(allowedDirectives);
			return (allowedDirectives);
		}
};

class	LocationBlock : public ABlock<LocationBlock>
{
	private:


	public:
		LocationBlock(int line, int column, const TStrVect& args) 
				: ABlock(line, column, args, Blocks::LOCATION)
		{}
		~LocationBlock(void) {}
};



// void	testDirectives(char **av)
// {
	// TStrVect args;
	// args.push_back(av[1]);
	// args.push_back(av[2]);
	// args.push_back(av[3]);
	// TStrVect	args1;
	// TStrVect	args2;
	// args1.push_back("404");
	// args1.push_back("405");
	// args1.push_back("406");
	// args1.push_back("407");
	// args1.push_back("/404.html");
	// args2.push_back("404");
	// args2.push_back("515");
	// args2.push_back("37");
	// args2.push_back("dpdpldpl");
	// args2.push_back("/418.html");
	// AConfigNode* node = new	Listen(1, 1, args);
	// AConfigNode* node = new ServerName(1, 1, args);
	// AConfigNode* node = new Alias(1, 1, args);
	// AConfigNode* node = new AllowedMethods(1, 1, args);
	// AConfigNode* node = new CgiPass(1, 1, args);
	// AConfigNode* node = new Root(1, 1, args);
	// AConfigNode* node = new Index(1, 1, args);
	// AConfigNode* node = new Autoindex(1, 1, args);
	// ErrorPage* node = new ErrorPage(1, 1, args1);
	// node->addErrorPages(args2);
	// AConfigNode* node = new ClientMaxBodySize(1, 1, args);
	// node->print(std::cout);
// }

int main(int ac, char **av)
{
	(void)ac;

	TStrVect	args;

	args.push_back(av[1]);

	ABlock<ServerBlock>*	block = new ServerBlock(0, 0, args);
	return (0);
}