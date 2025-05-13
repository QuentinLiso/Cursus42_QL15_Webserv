/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 19:32:00 by qliso             #+#    #+#             */
/*   Updated: 2025/05/13 18:12:58 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include <map>
#include <set>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdarg>
#include <cmath>
#include <climits>


std::vector<std::string>    split(const std::string &str, const std::string& delimiters)
{
    std::vector<std::string>    tokens;
    size_t                      start = str.find_first_not_of(delimiters);
    size_t                      end;

    while (start != std::string::npos)
    {
        end = str.find_first_of(delimiters, start);
        std::string token = str.substr(start, end - start);
        tokens.push_back(token);
        start = str.find_first_not_of(delimiters, end);
    }
    return (tokens);
}

std::string					fileToStr(const std::string& filename)
{
	std::ifstream	file(filename.c_str());
	std::ostringstream	buffer;
	
	if (!file)
		throw	std::runtime_error("Failed to open file: " + filename);
	buffer << file.rdbuf();
	file.close();
	return (buffer.str());
}


enum  TokenType
{
	TO_Identifier,
	TO_Number,
	TO_StringLiteral,
	TO_Semicolon,
	TO_OpenBrace,
	TO_CloseBrace,
	TO_EndOfFile,
	TO_Unknown
};	// TokenType
	
struct Token
{
    TokenType   type;
    std::string value;
    int         line;
    int         column;

    Token(TokenType t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c) {}
};	// Token

struct IConfigNode
{
    virtual			~IConfigNode(void) {};
    virtual bool	isBlock(void) const = 0;
};	// IConfigNode

struct Directive : public IConfigNode
{
    std::string                 name;
    std::vector<std::string>    arguments;

    Directive(const std::string& n, const std::vector<std::string>& args)
        : name(n), arguments(args) {}

    bool    isBlock(void) const { return false; }
};	// Directive

struct Block : public IConfigNode
{
    std::string                 name;
    std::vector<std::string>    arguments;
    std::vector<IConfigNode*>   children;

    Block(const std::string& n, const std::vector<std::string>& args) 
        : name(n), arguments(args) {}
    
    bool    isBlock(void) const { return true; }
};	// Block



class	Lexer
{
private:
	std::string			_input;
	size_t				_index;
	int					_line;
	int					_column;
	std::vector<Token>	_tokens;
	
	char	peekNextChar(void) const
	{
		if (_index >= _input.size()) 
			return ('\0');
		return (_input[_index]);
	}

	char	advanceNextChar(void)
	{
		if (_index >= _input.size())
			return ('\0');
		char	c = _input[_index++];
		if (c == '\n')
		{
			_line++;
			_column = 1;
		}
		else
			_column++;
		return (c);
	}

	void	skipWhitesAndComments(void)
	{
		while (true)
		{
			char	c = peekNextChar();
			if (std::isspace(c))
				advanceNextChar();
			else if (c == '#')
			{
				while (peekNextChar() != '\n' && peekNextChar() != '\0')
					advanceNextChar();
			}
			else
				break ;
		}
	}
	
	bool	isIdentifierChar(char c) const
	{
		std::string	specials = "_./-$:@=~^*\\";
		return (std::isalnum(c) || specials.find(c) != std::string::npos);
	}

	std::string	readIdentifier(void)
	{
		std::string		str;
		while (isIdentifierChar(peekNextChar()))
			str += advanceNextChar();
		return (str);
	}
	
	std::string	readQuotedString(char quote)
	{
		std::string		str;
		advanceNextChar();
		while (peekNextChar() != quote && peekNextChar() != '\0')
			str += advanceNextChar();
		if (peekNextChar() == '\0')
			throw std::runtime_error("Unterminated string literal");
		advanceNextChar();
		return (str);
	}

	Token		getNextToken(void)
	{
		skipWhitesAndComments();
		char	c = peekNextChar();
		int		startLine = _line;
		int		startCol = _column;

		if (c == '\0')
			return (Token(TO_EndOfFile, "", startLine, startCol));
		
		if (isIdentifierChar(c))
		{
			return (Token(TO_Identifier, readIdentifier(), startLine, startCol));
		}
		if (c == '"' || c == '\'')
		{
			return (Token(TO_StringLiteral, readQuotedString(c), startLine, startCol));
		}
		switch(c)
		{
			case	'{'	:
				advanceNextChar();
				return (Token(TO_OpenBrace, "{", startLine, startCol));
			case	'}'	:
				advanceNextChar();
				return (Token(TO_CloseBrace, "}", startLine, startCol));
			case	';'	:
				advanceNextChar();
				return (Token(TO_Semicolon, ";", startLine, startCol));
			default		:
				std::ostringstream err;
				err << "Unexpected character '" << c << "' at line " << _line << ", column " << _column;
				throw	std::runtime_error(err.str());
		}
	}

	void	tokenize(void)
	{
		while (true)
		{
			Token toki = getNextToken();
			_tokens.push_back(toki);
			if (toki.type == TO_EndOfFile)
				break ;
		}
	}

public:
	Lexer(const std::string& filename)
		: _index(0), _line(1), _column(1)
	{
		_input = fileToStr(filename);
		tokenize();
	}

	const std::vector<Token>&	getTokens(void) const { return _tokens; }
};	// Lexer

class	Parser
{
private:
	const std::vector<Token>&	_tokens;
	size_t						_index;
	std::vector<IConfigNode*>	_ast;

	bool	isEofToken(void) const
	{
		return (_index >= _tokens.size() || _tokens[_index].type == TO_EndOfFile);
	}

	Token	setStatement(std::vector<std::string>& args)
	{
		Token	statement = _tokens[_index++];
		while (!isEofToken() && _tokens[_index].type != TO_Semicolon && _tokens[_index].type != TO_OpenBrace)
		{
			const Token&	current = _tokens[_index];
			if (current.type != TO_Identifier && current.type != TO_StringLiteral)
				throw std::runtime_error("Not a valid arg token '" + _tokens[_index].value + "'");
			args.push_back(current.value);
			_index++;
		}
		return (statement);
	}

	IConfigNode*	buildSingleAst(void)
	{
		if (isEofToken() || _tokens[_index].type != TO_Identifier)
			throw std::runtime_error("Not an identifier token");
		
		std::vector<std::string>	args;
		Token						statement = setStatement(args);
		const Token&				current = _tokens[_index];
		
		if (current.type == TO_Semicolon)
		{
			_index++;
			return (new Directive(statement.value, args));
		}
		else if (current.type == TO_OpenBrace)
		{
			Block	*block = new Block(statement.value, args);
			_index++;
			while (!isEofToken() && _tokens[_index].type != TO_CloseBrace)
				block->children.push_back(buildSingleAst());
			if (isEofToken())
				throw std::runtime_error("Unclosed bracket");
			_index++;
			return (block);
		}
		throw std::runtime_error("Expected ';' or '{' after statement");
	}
	
	std::vector<IConfigNode*>	parseTokens(void)
	{
		std::vector<IConfigNode*>	ast;

		while (!isEofToken())
			ast.push_back(buildSingleAst());
		return (ast);
	}

	void	printNodes(const std::vector<IConfigNode*> node, size_t offset = 0) const
	{
		std::vector<IConfigNode*>::const_iterator	it = node.begin();
		std::vector<IConfigNode*>::const_iterator	ite = node.end();
		for (; it != ite; it++)
		{
			if ((*it)->isBlock())
			{
				const Block*	block = dynamic_cast<const Block*>(*it);
				if (!block)
					throw std::runtime_error("Failed to cast IConfigNode to Block");
				std::cout << "\t" << std::string(offset * 3, '-') << "Block : " << block->name;
				for (size_t	i = 0; i < block->arguments.size(); i++)
					std::cout << " " << block->arguments[i];
				std::cout << std::endl;
				offset++;
				printNodes(block->children, offset);
				offset--;
			}
			else
			{
				const Directive*	directive = dynamic_cast<const Directive*>(*it);
				if (!directive)
					throw std::runtime_error("Failed to cast IConfigNode to Directive");
				std::cout << "\t" << std::string(offset * 3, '-') << "Directive : " << directive->name;
				for (size_t	i = 0; i < directive->arguments.size(); i++)
					std::cout << " " << directive->arguments[i];
				std::cout << std::endl;
			}
		}
	}

	void	deleteNode(IConfigNode* node)
	{
		if (!node)
			return ;
		if (node->isBlock())
		{
			Block*	block = dynamic_cast<Block*>(node);
			if (block)
			{
				for (std::vector<IConfigNode*>::iterator it = block->children.begin(); it != block->children.end(); it++)
					deleteNode(*it);
			}
		}
		delete node;
	}

public:
	Parser(const std::vector<Token>& tokens)
		: _tokens(tokens), _index(0)
	{
		_ast = parseTokens();
	}
	
	virtual ~Parser(void)
	{
		for (std::vector<IConfigNode*>::iterator it = _ast.begin(); it != _ast.end(); it++)
			deleteNode(*it);
	}

	const std::vector<IConfigNode*>&	getAst(void) const { return _ast; }

	void	showAst(void) const
	{
		printNodes(_ast);
	}
};




enum	HttpMethods
{
	HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
	HTTP_PUT
};

struct	ServerConfig;

struct	LocationConfig
{
	// ServerConfig*				serverConfig;			// Location block specific
	std::string					pathModifier;			// Location block specific
	std::string					path;					// Location block specific
	std::string					root;					// Allowed in both server and location block
	std::string					alias;					// Location block specific
	std::vector<std::string>	index;					// Allowed in both server and location block
	std::pair<bool, bool>		autoindex;				// Allowed in both server and location block
	std::map<int, std::string>	errorPages;				// Allowed in both server and location block
	std::pair<bool, size_t>		clientMaxBodySize;		// Allowed in both server and location block
	std::set<HttpMethods>		allowedMethods;			// Location block specific
	std::string					cgiPass;				// Location block specific

};

struct	ServerConfig
{
	std::string					host;						// Server block specific
	int							port;						// Server block specific
	std::vector<std::string>	serverName;					// Server block specific
	std::string					root;						// Allowed in both server and location block
	std::vector<std::string>	index;						// Allowed in both server and location block
	std::pair<bool, bool>		autoindex;					// Allowed in both server and location block	
	std::map<int, std::string>	errorPages;					// Allowed in both server and location block
	std::pair<bool, size_t>		clientMaxBodySize;			// Allowed in both server and location block
	std::vector<LocationConfig>	locations;					// Server block specific

	ServerConfig() : host(""), port(-1), root("") {}
};


class	ServerConfigSetter
{
private:

	static long	strToVal(const std::string& val)
	{
		if (val.empty())
			return (-1);
		char*	end;
		long	l = strtol(val.c_str(), &end, 10);
		if (*end != '\0' || l < 0)
			return (-1);
		return (l);
	}

	static int		isValidPort(const std::string& port)
	{
		long	result = strToVal(port);
		if (result < 1 || result > 65535)
			throw std::runtime_error("Invalid port : " + port);
		return (static_cast<int>(result));
	}
	
	static std::string	isValidIp(const std::string& ip)
	{
		if (ip == "localhost")
			return ("127.0.0.1");
		std::vector<std::string>	fields = split(ip, ".");
		if (fields.size() != 4)
			throw std::runtime_error("Invalid IP address : " + ip);
		for (size_t	i = 0; i < 4; i++)
		{
			int	val = strToVal(fields[i]);
			if (val < 0 || val > 255)
				throw std::runtime_error("Invalid IP address : " + ip);
		}
		return (ip);
	}

	static void	makeHostAndPort(const Directive* directive, ServerConfig& config)
	{
		if (directive->arguments.size() != 1 || config.port != -1)
			throw std::runtime_error("Host error or doublon listen");
		
		std::string	listenArg = directive->arguments[0];
		std::size_t	sep = listenArg.find(':');
		if (sep == std::string::npos)
		{
			config.host = "0.0.0.0";
			config.port = isValidPort(listenArg);
		}
		else
		{
			config.host = isValidIp(listenArg.substr(0, sep));
			config.port = isValidPort(listenArg.substr(sep + 1));
		}
	}

	static void	makeServerName(const Directive* directive, ServerConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() == 0)
			throw std::runtime_error("Missing arguments in server_name directive");
		if (config.serverName.size() != 0)
		{
			std::cout << "WARNING : server_name already populated earlier, overwriting with current server_name" << std::endl;
			config.serverName.clear();
		}
		for (size_t i = 0; i < args.size(); i++)
		{
			if (!args[i].empty())
				config.serverName.push_back(args[i]);
		}
	}
	
	static void	makeServerRoot(const Directive* directive, ServerConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid argument in root directive");
		if (!config.root.empty())
			std::cout << "WARNING: root already defined earlier, overwriting with current directive" << std::endl;
		config.root = args[0];
	}

	template <typename T>
	static void	makeIndex(const Directive* directive, T& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() == 0)
			throw std::runtime_error("Missing arguments in index directive");
		if (config.index.size() != 0)
		{
			std::cout << "WARNING : index already populated earlier, overwriting with current directive" << std::endl;
			config.index.clear();
		}
		for (size_t i = 0; i < args.size(); i++)
		{
			if (!args[i].empty())
				config.index.push_back(args[i]);
		}
	}

	template <typename T>
	static void	makeAutoIndex(const Directive* directive, T& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1)
			throw std::runtime_error("Missing arguments in autoindex directive");
		if (config.autoindex.first == true)
			throw std::runtime_error("Duplicate autoindex directive found");
		if (args[0] == "on")
		{
			config.autoindex.first = true;
			config.autoindex.second = true;
		}
		else if (args[0] == "off")
		{
			config.autoindex.first = true;
			config.autoindex.second = false;
		}
		else
			throw std::runtime_error("Invalid arguments in autoindex directive : only 'on' and 'off' allowed");		
	}

	static bool	isValidErrorUri(const std::string& uri)
	{
		std::string	http = "http://";
		std::string https = "https://";
		
		if (uri.empty())
			return (false);
		if (uri[0] == '/' && uri.size() > 1)
			return (true);
		if (!uri.compare(0, http.size(), http) && (uri.size() > http.size()))
			return (true);
		if (!uri.compare(0, https.size(), https) && (uri.size() > https.size()))
			return (true);
		return (false);
	}

	template <typename T>
	static void	makeErrorPages(const Directive* directive, T& config)
	{
		std::vector<std::string>	args = directive->arguments;
		
		if (args.size() < 2)
			throw std::runtime_error("Missing arguments in error_pages directive");
		
		size_t	last = args.size() - 1;
		if (!isValidErrorUri(args[last]))
			throw std::runtime_error("Invalid URI in error_pages directive");
		for (size_t i = 0; i < last; i++)
		{
			int	error = strToVal(args[i]);
			if (error < 400 || error > 599)
				throw std::runtime_error("Invalid error code in error_pages directive");
			if (config.errorPages.find(error) != config.errorPages.end())
				std::cout << "WARNING : Error " << error << " already mapped in error_pages, overwritten by current directive" << std::endl;
			config.errorPages[error] = args[last];
		}
	}

	static size_t	strToValBytes(std::string const& val)
	{
		char*	end;

		size_t	maxMb = 100U;
		size_t	maxKb = maxMb * 1024U;
		size_t	maxBytes = maxKb * 1024U;
		
		size_t	convert = strtoul(val.c_str(), &end, 10);
		
		if (*end == '\0' && convert <= maxBytes)
			return (convert);
		else if ((*end == 'k' || *end == 'K') && *(end + 1) == '\0' && convert <= maxKb)
			return (convert * 1024U);
		else if ((*end == 'm' || *end == 'M') && *(end + 1) == '\0' && convert <= maxMb)
			return (convert * 1024U * 1024U);
		return (ULONG_MAX);
	}

	template < typename T >
	static void	makeClientMaxBodySize(const Directive* directive, T& config)
	{
		std::vector<std::string>	args = directive->arguments;

		if (args.size() != 1)
			throw std::runtime_error("Missing arguments in client_max_body_size directive");
		if (config.clientMaxBodySize.first == true)
			throw std::runtime_error("Duplicate client_max_body_size directive");
		size_t	bytes = strToValBytes(args[0]);
		if (bytes == ULONG_MAX)
			throw std::runtime_error("Invalid value '" + args[0] + "' in client_max_body_size directive");
		config.clientMaxBodySize.first = true;
		config.clientMaxBodySize.second = bytes;
	}

	static void	parseServerDirective(const Directive* directive, ServerConfig& config)
	{
		if (directive->name == "listen")
			makeHostAndPort(directive, config);
		else if (directive->name == "server_name")
			makeServerName(directive, config);
		else if (directive->name == "root")
			makeServerRoot(directive, config);
		else if (directive->name == "index")
			makeIndex(directive, config);
		else if (directive->name == "autoindex")
			makeAutoIndex(directive, config);
		else if (directive->name == "error_page")
			makeErrorPages(directive, config);
		else if (directive->name == "client_max_body_size")
			makeClientMaxBodySize(directive, config);
		else
			throw std::runtime_error("Unrecognized directive '" + directive->name + "' in server block");
		// std::cout << "Directive " + directive->name + " added to the server config" << std::endl;
	}
	
	static void	makeLocationRoot(const Directive* directive, LocationConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid argument in root directive");
		if (!config.alias.empty())
			throw std::runtime_error("Root directive aborted: Alias directive already defined in the location, cannot have both alias and root in the same location");
		if (!config.root.empty())
			std::cout << "WARNING: root already defined earlier, overwriting with current directive" << std::endl;
		config.root = args[0];
	}

	static void	makeLocationAlias(const Directive* directive, LocationConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid argument in root directive");
		if (!config.alias.empty() || !config.root.empty())
			throw std::runtime_error("Alias directive aborted: Alias or root directive already defined in the location, cannot have both alias and root in the same location");
		config.alias = args[0];
	}

	static void	makeAllowedMethods(const Directive* directive, LocationConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
	
		if (args.size() == 0)
			throw std::runtime_error("Missing arguments in allowed_methods directive");
		for (size_t i = 0; i < args.size(); i++)
		{
			if (args[i] == "GET")
				config.allowedMethods.insert(HTTP_GET);
			else if (args[i] == "PUT")
				config.allowedMethods.insert(HTTP_PUT);
			else if (args[i] == "DELETE")
				config.allowedMethods.insert(HTTP_DELETE);
			else if (args[i] == "POST")
				config.allowedMethods.insert(HTTP_POST);
			else
				throw std::runtime_error("Invalid argument '" + args[i] + "' in allowed_methods directive");
		}
	}

	static void	makeCgiPass(const Directive* directive, LocationConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;

		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("cgi_pass directive should have exactly 1 non-empty argument");
		if (!config.cgiPass.empty())
			throw std::runtime_error("Duplicate cgi_pass directive encountered");
		config.cgiPass = args[0];
	}

	static void	parseLocationDirective(const Directive* directive, LocationConfig& config)
	{
		if(directive->name == "root")
			makeLocationRoot(directive, config);
		else if (directive->name == "alias")
			makeLocationAlias(directive, config);
		else if (directive->name == "index")
			makeIndex(directive, config);
		else if (directive->name == "autoindex")
			makeAutoIndex(directive, config);
		else if (directive->name == "error_page")
			makeErrorPages(directive, config);
		else if (directive->name == "client_max_body_size")
			makeClientMaxBodySize(directive, config);
		else if (directive->name == "allowed_methods")
			makeAllowedMethods(directive, config);
		else if (directive->name == "cgi_pass")
			makeCgiPass(directive, config);

		// std::cout << "Directive " + directive->name + " added to the location config" << std::endl;
	}

	static void	parseLocationBlockArgs(const Block* block, LocationConfig& locConfig)
	{
		std::vector<std::string>	args = block->arguments;
		
		switch(args.size())
		{
			case 1:
				if (args[0].empty() || args[0][0] != '/')
					throw std::runtime_error("Invalid path in location args");
				locConfig.path = args[0];
				break;
			case 2:
				if (args[0] != "=" && args[0] != "~" && args[0] != "~*" && args[0] != "^~")
					throw std::runtime_error("Invalid modifier '" + args[0] + "' in location args");
				if (args[1].empty() || ( (args[1][0] != '/') && (args[0] == "=" || args[0] == "^~") ))
					throw std::runtime_error("Invalid path in location args");
				locConfig.path = args[1];
				locConfig.pathModifier = args[0];
				break;
			default:
				throw std::runtime_error("Invalid args in location block");
				break;
		}
	
	}

	static void	inheritanceLocationBlock(LocationConfig& locConfig, ServerConfig& serverConfig)
	{
		// if (locConfig.serverConfig == NULL)
		// 	return ;
		// ServerConfig*	serverConfig = locConfig.serverConfig;

		if (locConfig.root.empty() && locConfig.alias.empty())
			locConfig.root = serverConfig.root;
		if (locConfig.index.empty())
			locConfig.index = serverConfig.index;
		if (locConfig.autoindex.first == false)
			locConfig.autoindex = serverConfig.autoindex;
		if (locConfig.errorPages.empty())
			locConfig.errorPages = serverConfig.errorPages;
		if (locConfig.clientMaxBodySize.first == false)
			locConfig.clientMaxBodySize = serverConfig.clientMaxBodySize;
	}

	static void	parseLocationBlock(const Block* block, ServerConfig& config)
	{
		LocationConfig	locConfig;
		
		// locConfig.serverConfig = &config;
		parseLocationBlockArgs(block, locConfig);
		for (size_t i = 0; i < block->children.size(); i++)
		{
			IConfigNode*	child = block->children[i];
			// std::cout << "Location block child n°" << i << " :\t";
			if (child->isBlock())
				throw std::runtime_error("Block found in location block");
			
			Directive*	childDirective = dynamic_cast<Directive*>(child);
			if (!childDirective)
				throw std::runtime_error("Dynamic cast childDirective failed");
			parseLocationDirective(childDirective, locConfig);
		}
		inheritanceLocationBlock(locConfig, config);
		config.locations.push_back(locConfig);
		// std::cout << "Location block added to the server config" << std::endl;
	}

public:
	static ServerConfig	parseServerBlock(const Block* block)
	{
		ServerConfig	serverConfig;

		if (block->arguments.size() != 0)
			throw std::runtime_error("Server block cannot have arguments");
		for (size_t i = 0; i < block->children.size(); i++)
		{
			IConfigNode*	child = block->children[i];
			// std::cout << "Server block children n°" << i << " :\t";
			if (child->isBlock())
			{
				Block*	childBlock = dynamic_cast<Block*>(child);
				if (!childBlock)
					throw std::runtime_error("Dynamic cast childBlock failed");
				if (childBlock->name != "location")
					throw std::runtime_error("Unrecognized location block inside server");
				parseLocationBlock(childBlock, serverConfig);
				continue ;
			}
			Directive*	childDirective = dynamic_cast<Directive*>(child);
			if (!childDirective)
				throw std::runtime_error("Dynamic cast childDirective failed");
			parseServerDirective(childDirective, serverConfig);
		}
		return (serverConfig);
		// std::cout << "Server config added to the manager vector of servers" << std::endl;
	}
	
	static void	printLocationConfig(const LocationConfig& config, size_t i)
	{
		std::cout 	<< "\t****** LOCATION BLOCK " << i << " ******\n" 
					<< "\tPath modifier : " + config.pathModifier + "\n"
					<< "\tPath : " + config.path + "\n"
					<< "\tRoot : " + config.root + "\n"
					<< "\tAlias : " + config.alias + "\n"
					<< "\tIndex : ";
		for (size_t i = 0; i < config.index.size(); i++)
			std::cout << config.index[i] << " ";
		std::cout 	<< "\n"
				 	<< "\tAutoindex : " << ((config.autoindex.second == true) ? "on" : "off") << "\n"
					<< "\tError pages :\n";
		for (std::map<int, std::string>::const_iterator it = config.errorPages.begin(); it != config.errorPages.end(); it++)
			std::cout << "\t\t" << it->first << " : " << it->second << "\n";
		std::cout 	<< "\tClient max body size (in bytes) : " << config.clientMaxBodySize.second << "\n"
					<< "\tAllowed methods (1: GET, 2: POST, 3: DELETE, 4: PUT) : ";
		for (std::set<HttpMethods>::const_iterator it = config.allowedMethods.begin(); it != config.allowedMethods.end(); it++)
			std::cout << (*it + 1) << " ";
		std::cout	<< "\n"
					<< "\tCGI Pass : " + config.cgiPass
					<< "\n" << std::endl;
	}
	
	static void	printServerConfig(const ServerConfig& config, size_t i)
	{
		std::cout 	<< "****** SERVER BLOCK " << i << "******\n" 
					<< "Host : " + config.host + "\n"
					<< "Port : " << config.port << "\n"
					<< "Server name : ";
		for (size_t i = 0; i < config.serverName.size(); i++)
			std::cout << config.serverName[i] << " ";
		std::cout 	<< "\n"
					<< "Root : " + config.root + "\n"
					<< "Index : ";
		for (size_t i = 0; i < config.index.size(); i++)
			std::cout << config.index[i] << " ";
		std::cout 	<< "\n"
				 	<< "Autoindex : " << ((config.autoindex.second == true) ? "on" : "off") << "\n"
					<< "Error pages :\n";
		for (std::map<int, std::string>::const_iterator it = config.errorPages.begin(); it != config.errorPages.end(); it++)
			std::cout << "\t" << it->first << " : " << it->second << "\n";
		std::cout 	<< "Client max body size (in bytes) : " << config.clientMaxBodySize.second << "\n";
		for (size_t i = 0; i < config.locations.size(); i++)
			printLocationConfig(config.locations[i], i);
		std::cout	<< "\n" << std::endl;
	}
};


class	ConfigBuilder
{
private:
	std::vector<ServerConfig>	_servers;

	void	validateServerConfig(const ServerConfig& config) {}
	void	validateLocationConfig(const LocationConfig& config) {}

	void	loadFromAst(const std::vector<IConfigNode*>& ast)
	{
		for (std::vector<IConfigNode*>::const_iterator it = ast.begin(); it != ast.end(); it++)
		{
			
			if ((*it)->isBlock())
			{
				Block*	block = dynamic_cast<Block*>(*it);
				if (!block)
					throw std::runtime_error("Dynamic cast to block failed");
				if (block->name != "server")
					throw std::runtime_error("Expected 'server' block at top level");
				_servers.push_back(ServerConfigSetter::parseServerBlock(block));
				continue ;
			}
			throw std::runtime_error("Unrecognized server block at top ast level");
		}
	}


public:
	ConfigBuilder(const std::vector<IConfigNode*>& ast)
	{
		loadFromAst(ast);
	}

	~ConfigBuilder(void) {}

	const std::vector<ServerConfig>&	getServers(void) const { return (_servers); }

	void	printConfig(void) const
	{
		for (size_t i = 0; i < _servers.size(); i++)
			ServerConfigSetter::printServerConfig(_servers[i], i);
	}
};



int main(void)
{
	Lexer		lexer("lol.conf");
	Parser		parser(lexer.getTokens());
	parser.showAst();
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;

	ConfigBuilder	config(parser.getAst());
	config.printConfig();
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;
	    
	return (0);
}