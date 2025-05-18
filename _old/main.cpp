/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 19:32:00 by qliso             #+#    #+#             */
/*   Updated: 2025/05/16 23:37:42 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdarg>
#include <cmath>
#include <climits>
#include <sys/stat.h>
#include <unistd.h>


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

template < typename T>
std::string	convToStr(T& val)
{
	std::ostringstream	oss;
	oss << val;
	return (oss.str());
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

struct	LocationConfig
{
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
	std::vector<std::string>	serverNames;					// Server block specific
	std::string					root;						// Allowed in both server and location block
	std::vector<std::string>	index;						// Allowed in both server and location block
	std::pair<bool, bool>		autoindex;					// Allowed in both server and location block	
	std::map<int, std::string>	errorPages;					// Allowed in both server and location block
	std::pair<bool, size_t>		clientMaxBodySize;			// Allowed in both server and location block
	std::vector<LocationConfig>	locations;					// Server block specific

	ServerConfig() : host(""), port(-1), root("") {}
	
	const LocationConfig*	getDefaultLocationBlock(void) const
	{
		for (size_t i = 0; i < locations.size(); i++)
		{
			if (locations[i].path == "/")
				return (&locations[i]);
		}
		return (NULL);
	}
};

class	ServerConfigSetter
{
private:
	const static std::string	_rootAliasSpecials;

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

	static void	makeServerNames(const Directive* directive, ServerConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() == 0)
			throw std::runtime_error("Missing arguments in server_name directive");
		if (config.serverNames.size() != 0)
		{
			std::cout << "WARNING : server_name already populated earlier, overwriting with current server_name" << std::endl;
			config.serverNames.clear();
		}
		for (size_t i = 0; i < args.size(); i++)
		{
			if (!args[i].empty())
				config.serverNames.push_back(args[i]);
		}
	}
	
	
	static bool	isValidRootAliasChar(char c)
	{
		return (std::isalnum(c) || _rootAliasSpecials.find(c) != std::string::npos);
	}

	static	std::string&	removeTrailingChar(std::string& str, char c)
	{
		if (str.empty())
			return (str);
		while (str.size() > 1 && str[str.size() - 1] == c)
			str.erase(str.size() - 1);
		return (str);
	}

	static std::string&		removeDuplicateChar(std::string& str, char c)
	{
		std::string doublon = std::string(2, c);
		size_t	i = str.find(doublon);
		if (i == std::string::npos)
			return (str);
		std::cout << "WARNING: double slash '//' found in directive : '" + str + "'";
		while (str.size() > 1 && i != std::string::npos)
		{
			str.erase(i + 1, 1);
			i = str.find(doublon);
		}
		std::cout << " -> Normalized to '" + str + "'" << std::endl;
		return (str);
	}

	static bool	containsDoubleDotsAccess(const std::string& str)
	{
		std::vector<std::string>	segments;
		std::istringstream			iss(str);
		std::string					segment;

		while (std::getline(iss, segment, '/'))
		{
			if (segment == "..")
					return (true); 
		}
		return (false);
	}

	static std::string&		removeDotPaths(std::string& str)
	{
		std::vector<std::string>	segments;
		std::istringstream			iss(str);
		std::string					segment;

		while (std::getline(iss, segment, '/'))
		{
			if (segment != "." && !segment.empty())
				segments.push_back(segment);
		}

		str.clear();
		for (size_t i = 0; i < segments.size(); i++)
			str += "/" + segments[i];
		
		if (str.empty())
			str = "/";
		return (str);
	}

	static std::string&		normalizeRootAlias(std::string& str)
	{
		removeDuplicateChar(str, '/');
		removeTrailingChar(str, '/');
		if (containsDoubleDotsAccess(str))
			throw std::runtime_error("Filepath " + str + " contains forbidden '..' access");
		removeDotPaths(str);
		return (str);
	}



	static void	makeServerRoot(const Directive* directive, ServerConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid argument in root directive");
		
		std::string	root = args[0];
		if (root[0] != '/')
			throw std::runtime_error("root directive '" + root + "' must start with a '/'");
		for (size_t i = 0; i < root.size(); i++)
		{
			if (!isValidRootAliasChar(root[i]))
				throw std::runtime_error("Invalid character found in root directive '" + root + "'");
		}
		if (!config.root.empty())
			std::cout << "WARNING: root already defined earlier, overwriting with current directive" << std::endl;	
		config.root = normalizeRootAlias(root);
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
			std::string	fileName = args[i];
			if (fileName.empty() || fileName.find('/') != std::string::npos || fileName == "." || fileName == "..")
				throw std::runtime_error("Invalid index filename '" + fileName + "'");
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
			throw std::runtime_error("Invalid arguments in autoindex directive : only 'on' and 'off' allowed, and we got '" + args[0] + "'");		
	}

	static void	checkValidErrorUri(const std::string& uri)
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

	template <typename T>
	static void	makeErrorPages(const Directive* directive, T& config)
	{
		std::vector<std::string>	args = directive->arguments;
		
		if (args.size() < 2)
			throw std::runtime_error("Missing arguments in error_pages directive");
		
		size_t	last = args.size() - 1;
		checkValidErrorUri(args[last]);
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
			makeServerNames(directive, config);
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
		
		std::string	root = args[0];
		if (root[0] != '/')
			throw std::runtime_error("root directive '" + root + "' must start with a '/'");
		for (size_t i = 0; i < root.size(); i++)
		{
			if (!isValidRootAliasChar(root[i]))
				throw std::runtime_error("Invalid character found in root directive '" + root + "'");
		}
		
		if (!config.root.empty())
			std::cout << "WARNING: root already defined earlier, overwriting with current directive" << std::endl;

		config.root = normalizeRootAlias(root);
	}

	static void	makeLocationAlias(const Directive* directive, LocationConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid argument in root directive");
		if (!config.alias.empty() || !config.root.empty())
			throw std::runtime_error("Alias directive aborted: Alias or root directive already defined in the location, cannot have both alias and root in the same location");
		
		std::string	alias = args[0];
		if (alias[0] != '/')
			throw std::runtime_error("alias directive '" + alias + "' must start with a '/'");
		for (size_t i = 0; i < alias.size(); i++)
		{
			if (!isValidRootAliasChar(alias[i]))
				throw std::runtime_error("Invalid character found in root directive '" + alias + "'");
		}

		config.alias = normalizeRootAlias(alias);
	}

	static void	addAllowedMethod(std::set<HttpMethods>& configHttpMethods, HttpMethods methodId, const std::string& methodStr)
	{
		if (configHttpMethods.insert(methodId).second == false)
			std::cout << "WARNING: Duplicate HTTP method '" << methodStr << "' found in allowed_methods directive" << std::endl;
	}

	static void	makeAllowedMethods(const Directive* directive, LocationConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
	
		if (args.size() == 0)
			throw std::runtime_error("Missing arguments in allowed_methods directive");
		if (!config.allowedMethods.empty())
			throw std::runtime_error("Duplicate allowed_methods directive found in location config '" + config.path + "'");
		for (size_t i = 0; i < args.size(); i++)
		{
			std::string&	method = args[i];
			if (method == "GET")
				addAllowedMethod(config.allowedMethods, HTTP_GET, method);
			else if (method == "PUT")
				addAllowedMethod(config.allowedMethods, HTTP_PUT, method);
			else if (method == "DELETE")
				addAllowedMethod(config.allowedMethods, HTTP_DELETE, method);
			else if (method == "POST")
				addAllowedMethod(config.allowedMethods, HTTP_POST, method);
			else
				throw std::runtime_error("Invalid argument '" + method + "' in allowed_methods directive");
		}
	}

	static void	makeCgiPass(const Directive* directive, LocationConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;

		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("cgi_pass directive should have exactly 1 non-empty argument");
		if (!config.cgiPass.empty())
			throw std::runtime_error("Duplicate cgi_pass directive encountered");
		if (args[0][0] != '/')
			throw std::runtime_error("cgi_pass '" + args[0] + "'must be an absolute path, so must start with '/'");
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
		
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid path argument in location statement");
		
		std::string	path = args[0];
		if (path[0] != '/')
			throw std::runtime_error("Location path argument '" + path + "' must start with a '/'");
		for (size_t i = 0; i < path.size(); i++)
		{
			if (!isValidRootAliasChar(path[i]))
				throw std::runtime_error("Invalid character found in location path '" + path + "'");
		}
		removeDuplicateChar(path, '/');
		if (containsDoubleDotsAccess(path))
			throw std::runtime_error("Filepath " + path + " contains forbidden '..' access");
		locConfig.path = path;
	}

	static void	inheritanceLocationBlock(LocationConfig& locConfig, ServerConfig& serverConfig)
	{
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

	static void	addFallbackLocationBlock(ServerConfig& serverConfig)
	{
		LocationConfig	locConfig;
	
		locConfig.pathModifier = "";
		locConfig.path = "/";
		locConfig.root = serverConfig.root;
		locConfig.alias = "";
		locConfig.index = serverConfig.index;
		locConfig.autoindex = serverConfig.autoindex;
		locConfig.errorPages = serverConfig.errorPages;
		locConfig.clientMaxBodySize = serverConfig.clientMaxBodySize;
		locConfig.allowedMethods.insert(HTTP_GET);
		locConfig.allowedMethods.insert(HTTP_POST);
		locConfig.cgiPass = "";
		
		serverConfig.locations.push_back(locConfig);
	}

	static bool	hasDefaultLocationBlock(ServerConfig& serverConfig)
	{
		const std::vector<LocationConfig>& locations = serverConfig.locations;
		for (size_t i = 0; i < locations.size(); i++)
		{
			if (locations[i].path == "/")
				return (true);
		}
		return (false);
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
		if (!hasDefaultLocationBlock(serverConfig))
			addFallbackLocationBlock(serverConfig);
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
		for (size_t j = 0; j < config.index.size(); i++)
			std::cout << config.index[j] << " ";
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
		for (size_t i = 0; i < config.serverNames.size(); i++)
			std::cout << config.serverNames[i] << " ";
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

const std::string	ServerConfigSetter::_rootAliasSpecials = "/-_.";


class	ConfigBuilder
{
public:
	typedef std::map<std::pair<std::string, int>, std::set<std::string> >	IpPortDomainsMap;

private:
	std::vector<ServerConfig>	_servers;

	std::vector<ServerConfig>	loadFromAst(const std::vector<IConfigNode*>& ast)
	{
		std::vector<ServerConfig>	servers;
		for (std::vector<IConfigNode*>::const_iterator it = ast.begin(); it != ast.end(); it++)
		{
			
			if ((*it)->isBlock())
			{
				Block*	block = dynamic_cast<Block*>(*it);
				if (!block)
					throw std::runtime_error("Dynamic cast to block failed");
				if (block->name != "server")
					throw std::runtime_error("Expected 'server' block at top level");
				servers.push_back(ServerConfigSetter::parseServerBlock(block));
				continue ;
			}
			throw std::runtime_error("Unrecognized server block at top ast level");
		}
		return (servers);
	}

	void	validateIpPortDomains(void) const
	{
		IpPortDomainsMap	ipPortDomainsMap;			// std::map<std::pair<std::string, int>, std::set<std::string> >		=	map < <ip, port>, domains >
		
		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig&	server = _servers[i];
			if (server.port == -1)
				throw std::runtime_error("server has no defined port");
			
			std::pair<std::string, int> ipPort(server.host, server.port);			// Create a pair with the server host and server port
			std::set<std::string>&	ipPortDomains = ipPortDomainsMap[ipPort];		// Look for the vector of domains at the key ip/port inside the ipPortDomainMap
			
			for (size_t j = 0; j < server.serverNames.size(); j++)
			{
				const std::string&	serverName = server.serverNames[j];
				if (ipPortDomains.insert(serverName).second == false)				// Throw if the given serverName is found in the vector of domains at the key ip/port
					throw std::runtime_error("Duplicate server name " + serverName + " for host " + server.host + ":" + convToStr(server.port));
			}

			if (server.serverNames.empty() && !ipPortDomains.empty())
				throw std::runtime_error("Server block with no server_name directive for host " + server.host + ":" + convToStr(server.port) + " must be declared first in config file");
		}
	}

	void	checkZeroPorts(void) const
	{
		const std::vector<const ServerConfig*> zerosConfigs = findConfigs("0.0.0.0");
		if (zerosConfigs.empty())
			return ;
	
		std::set<int>	zeroPorts;
		for (size_t i = 0; i < zerosConfigs.size(); i++)
		{
			if (zeroPorts.insert(zerosConfigs[i]->port).second == false)
				throw std::runtime_error("Duplicate listening port " + convToStr(zerosConfigs[i]->port) + " on default ip 0.0.0.0");
		}
		
		const std::vector<const ServerConfig*> nonZerosConfigs = findOtherConfigs("0.0.0.0");
		for (size_t i = 0; i < nonZerosConfigs.size(); i++)
		{
			const ServerConfig* config = nonZerosConfigs[i];
			if (zeroPorts.find(config->port) != zeroPorts.end())
				throw std::runtime_error("Duplicate listening port " + convToStr(config->port) + " on default ip 0.0.0.0 and ip " + config->host);
		}
	}

	void	checkServerEmptyRoot(const ServerConfig& server) const
	{
		bool	hasDefaultLocation = false;
		for (size_t j = 0; j < server.locations.size(); j++)
		{
			const LocationConfig&	location = server.locations[j];
			if (location.path == "/")
				hasDefaultLocation = true;
			if (location.root.empty() && location.alias.empty())
				throw std::runtime_error("Empty root at server level is not covered by root or alias at location level '" + location.path + "'");
		}
		if (!hasDefaultLocation)
			throw std::runtime_error("Server with empty root has no default 'location /' path");
	}

	void	checkDuplicatePaths(const ServerConfig& server) const
	{
		std::set<std::string>	paths;
		
		for (size_t j = 0; j < server.locations.size(); j++)
		{
			const std::string& path = server.locations[j].path;
			if (paths.insert(path).second == false)
				throw std::runtime_error("Duplicate location path '" + path + "' found in same server block");
		}
	}

	void	checkExecutableDirectory(const std::string& folderPath) const
	{
		struct stat	fileStatus;
		int	status = stat(folderPath.c_str(), &fileStatus);
		
		if (status != 0)
		{
			std::cout << "WARNING: " << folderPath << " does not exist yet" << std::endl;
			return ;
		}
			
		if (!S_ISDIR(fileStatus.st_mode))
		{
			std::cout << "WARNING: " << folderPath << " exists but is not a directory" << std::endl;
			return ;
		}
			
		if (access(folderPath.c_str(), X_OK | R_OK))
		{
			std::cout << "WARNING: " << folderPath << " cannot be accessed" << std::endl;
			return ;
		}
	}

	std::string	joinPaths(const std::string& a, const std::string& b) const
	{
		if(a.empty())
			return (b);
		if (b.empty())
			return (a);
		if (a[a.size() - 1] == '/' && b[0] == '/')
			return (a + b.substr(1));
		else if (a[a.size() - 1] != '/' && b[0] != '/')
			return (a + "/" + b);
		else
			return (a + b);
	}

	void	checkAccessRights(const ServerConfig& server) const
	{
		if (!server.root.empty())
			checkExecutableDirectory(server.root);
		for (size_t i = 0; i < server.locations.size(); i++)
		{
			const LocationConfig& locConfig = server.locations[i];
			if (!locConfig.root.empty())
				checkExecutableDirectory(joinPaths(locConfig.root, locConfig.path));
			else
				checkExecutableDirectory(locConfig.alias);
		}
	}

	void	checkPathsRootAlias(const ServerConfig& server) const
	{
		if (server.root.empty())
			checkServerEmptyRoot(server);
		checkDuplicatePaths(server);
		checkAccessRights(server);
	}
	
	void	fillEmptyAllowedMethods(ServerConfig& server)
	{
		for (size_t i = 0; i < server.locations.size(); i++)
		{
			LocationConfig& locConfig = server.locations[i];
			if (locConfig.allowedMethods.empty())
			{
				std::cout << "WARNING: location '" + locConfig.path + "' has no allowed_methods directive, adding GET and POST by default" << std::endl;
				locConfig.allowedMethods.insert(HTTP_GET);
				locConfig.allowedMethods.insert(HTTP_POST);
			}
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

	void	checkCgiAccessrights(const ServerConfig& server) const
	{
		for (size_t i = 0; i < server.locations.size(); i++)
		{
			const std::string& cgiPass = server.locations[i].cgiPass;
			if (!cgiPass.empty())
			{
				checkCgiBin(cgiPass);
				checkCgiDirectory(cgiPass);
			}
		}
	}

	void	serversLoopCheck(void)
	{
		for (size_t i = 0; i < _servers.size(); i++)
		{
			ServerConfig& server = _servers[i];
			
			checkPathsRootAlias(server);
			fillEmptyAllowedMethods(server);
			checkCgiAccessrights(server);
		}
	}


public:

	ConfigBuilder(const std::vector<IConfigNode*>& ast)
	{
		_servers = loadFromAst(ast);
		validateIpPortDomains();
		checkZeroPorts();
		serversLoopCheck();
	}

	~ConfigBuilder(void) {}

	const std::vector<ServerConfig>&	getServers(void) const { return (_servers); }

	const std::vector<const ServerConfig*>	findConfigs(int port) const
	{
		std::vector<const ServerConfig*>	configs;

		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.port == port)
				configs.push_back(&server);
		}
		return (configs);
	}

	const std::vector<const ServerConfig*>	findConfigs(const std::string& ip, bool exactMatch = true) const
	{
		std::vector<const ServerConfig*>	configs;

		if (exactMatch)
		{
			for (size_t i = 0; i < _servers.size(); i++)
			{
				const ServerConfig& server = _servers[i];
				if (server.host == ip)
					configs.push_back(&server);
			}
		}
		else
		{
			for (size_t i = 0; i < _servers.size(); i++)
			{
				const ServerConfig& server = _servers[i];
				if (server.host.find(ip) != std::string::npos)
					configs.push_back(&server);
			}
		}
		return (configs);
	}

	const std::vector<const ServerConfig*>	findConfigs(const std::string& ip, int port, bool exactMatch = true) const
	{
		std::vector<const ServerConfig*>	configs;

		if (exactMatch)
		{
			for (size_t i = 0; i < _servers.size(); i++)
			{
				const ServerConfig& server = _servers[i];
				if (server.host == ip && server.port == port)
					configs.push_back(&server);
			}
		}
		else
		{
			for (size_t i = 0; i < _servers.size(); i++)
			{
				const ServerConfig& server = _servers[i];
				if (server.host.find(ip) != std::string::npos && server.port == port)
					configs.push_back(&server);
			}
		}
		return (configs);
	}

	const std::vector<const ServerConfig*>	findOtherConfigs(int port) const
	{
		std::vector<const ServerConfig*>	configs;

		for (size_t i = 0; i < _servers.size(); i++)
		{
			const ServerConfig& server = _servers[i];
			if (server.port != port)
				configs.push_back(&server);
		}
		return (configs);
	}

	const std::vector<const ServerConfig*>	findOtherConfigs(const std::string& ip, bool exactMatch = true) const
	{
		std::vector<const ServerConfig*>	configs;

		if (exactMatch)
		{
			for (size_t i = 0; i < _servers.size(); i++)
			{
				const ServerConfig& server = _servers[i];
				if (server.host != ip)
					configs.push_back(&server);
			}
		}
		else
		{
			for (size_t i = 0; i < _servers.size(); i++)
			{
				const ServerConfig& server = _servers[i];
				if (server.host.find(ip) == std::string::npos)
					configs.push_back(&server);
			}
		}
		return (configs);
	}

	const std::vector<const ServerConfig*>	findOtherConfigs(const std::string& ip, int port, bool exactMatch = true) const
	{
		std::vector<const ServerConfig*>	configs;

		if (exactMatch)
		{
			for (size_t i = 0; i < _servers.size(); i++)
			{
				const ServerConfig& server = _servers[i];
				if (server.host != ip || server.port != port)
					configs.push_back(&server);
			}
		}
		else
		{
			for (size_t i = 0; i < _servers.size(); i++)
			{
				const ServerConfig& server = _servers[i];
				if (server.host.find(ip) != std::string::npos || server.port != port)
					configs.push_back(&server);
			}
		}
		return (configs);
	}

	void	printConfig(const std::vector<ServerConfig>& servers) const
	{
		for (size_t i = 0; i < servers.size(); i++)
			ServerConfigSetter::printServerConfig(servers[i], i);
	}

	void	printConfig(const std::vector<const ServerConfig*> servers) const
	{
		for (size_t i = 0; i < servers.size(); i++)
		{
			if (servers[i])
				ServerConfigSetter::printServerConfig(*(servers[i]), i);
		}
	}

	void	printConfig(const std::string& ip, int port, const std::string& domain) const
	{
		const ServerConfig*	config = findServerConfig(ip, port, domain);
		if (config)
			ServerConfigSetter::printServerConfig(*config, 0);
		else
			std::cout << "No config found for ip '" << ip << "', port '" << port << "', domain '" << domain << "'" << std::endl;
	}

	void	printConfig(const std::string& ip, int port, const std::string& domain, const std::string& uri) const
	{
		const LocationConfig*	config = findLocationConfig(ip, port, domain, uri);
		if (config)
			ServerConfigSetter::printLocationConfig(*config, 0);
		else
			std::cout << "No config found for ip '" << ip << "', port '" << port << "', domain '" << domain << "', uri '" << uri << "'" << std::endl;
	}

	const	ServerConfig*	findServerConfig(const std::string& ip, int port, const std::string& domain) const
	{
		std::vector<const ServerConfig*>	configs = findConfigs(ip, port);

		if (configs.empty())			// NO config for this ip/port
			return (NULL);
		if (domain.empty())				// There are some config but domain is empty so we look for a default config, which can only be the first one
			return (configs[0]);

		for (size_t i = 0; i < configs.size(); i++)
		{
			const std::vector<std::string>& configServerNames = configs[i]->serverNames;
			if (std::find(configServerNames.begin(), configServerNames.end(), domain) != configServerNames.end())
				return (configs[i]);	// We found the domain in the vector of serverNames for a given ip/port (uniqueness was checked during validation)
		}
		return (configs[0]);			// Fallback to first config if no domain match but we had found config(s) for this ip/port
	}

	const	LocationConfig*	findLocationConfig(const std::string& ip, int port, const std::string& domain, const std::string& uri) const
	{
		const ServerConfig*	serverConfig = findServerConfig(ip, port, domain);
		if (!serverConfig || uri.empty() || uri[0] != '/')
			return (NULL);
		
		const std::vector<LocationConfig>& locations = serverConfig->locations;
		const LocationConfig* bestMatch = serverConfig->getDefaultLocationBlock();
		if (!bestMatch)
			return (NULL);

		for (size_t i = 0; i < locations.size(); i++)
		{
			const std::string& path = locations[i].path;
			if (uri.compare(0, path.size(), path) == 0 && path.size() > bestMatch->path.size())
				bestMatch = &locations[i];
		}
		return (bestMatch);
	}

};

	
int main(void)
{
	Lexer		lexer("lol2.conf");
	Parser		parser(lexer.getTokens());
	parser.showAst();
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;

	ConfigBuilder	config(parser.getAst());
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;

	config.printConfig(config.getServers());
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;
	    
	config.printConfig("127.0.0.1", 80, "lil.com", "/hello/");
	// config.printConfig(config.findOtherConfigs("0.0.0.0", 81));
	// std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;
	    
	// config.printConfig("127.0.0.2", 80, "zizou.com");

	return (0);
}