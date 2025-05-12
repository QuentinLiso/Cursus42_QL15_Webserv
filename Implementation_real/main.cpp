/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 19:32:00 by qliso             #+#    #+#             */
/*   Updated: 2025/05/11 23:53:15 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <vector>
#include <map>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cstdarg>
#include <cmath>


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

enum class  TokenType
{
	Identifier,
	Number,
	StringLiteral,
	Semicolon,
	OpenBrace,
	CloseBrace,
	EndOfFile,
	Unknown
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

class ConfigFile
{
private:
	std::string	_content;
	
public:
	ConfigFile(const std::string& filename)
	{
		std::ifstream	file(filename.c_str());
		if (!file)
			throw	std::runtime_error("Failed to open file: " + filename);
		std::ostringstream	buffer;
		buffer << file.rdbuf();
		_content = buffer.str();
		file.close();
	}
	const std::string&	getContent(void) const { return _content; }
};	// ConfigFile

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
		std::string	specials = "_./-$:@^*";
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
			return (Token(TokenType::EndOfFile, "", startLine, startCol));
		
		if (isIdentifierChar(c))
		{
			return (Token(TokenType::Identifier, readIdentifier(), startLine, startCol));
		}
		if (c == '"' || c == '\'')
		{
			return (Token(TokenType::StringLiteral, readQuotedString(c), startLine, startCol));
		}
		switch(c)
		{
			case	'{'	:
				advanceNextChar();
				return (Token(TokenType::OpenBrace, "{", startLine, startCol));
			case	'}'	:
				advanceNextChar();
				return (Token(TokenType::CloseBrace, "}", startLine, startCol));
			case	';'	:
				advanceNextChar();
				return (Token(TokenType::Semicolon, ";", startLine, startCol));
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
			if (toki.type == TokenType::EndOfFile)
				break ;
		}
	}

public:
	Lexer(const std::string& input)
		: _input(input), _index(0), _line(1), _column(1)
	{
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
		return (_index >= _tokens.size() || _tokens[_index].type == TokenType::EndOfFile);
	}

	Token	setStatement(std::vector<std::string>& args)
	{
		Token	statement = _tokens[_index++];
		while (!isEofToken() && _tokens[_index].type != TokenType::Semicolon && _tokens[_index].type != TokenType::OpenBrace)
		{
			const Token&	current = _tokens[_index];
			if (current.type != TokenType::Identifier && current.type != TokenType::StringLiteral)
				throw std::runtime_error("Not a valid arg token '" + _tokens[_index].value + "'");
			args.push_back(current.value);
			_index++;
		}
		return (statement);
	}

	IConfigNode*	buildSingleAst(void)
	{
		if (isEofToken() || _tokens[_index].type != TokenType::Identifier)
			throw std::runtime_error("Not an identifier token");
		
		std::vector<std::string>	args;
		Token						statement = setStatement(args);
		const Token&				current = _tokens[_index];
		
		if (current.type == TokenType::Semicolon)
		{
			_index++;
			return (new Directive(statement.value, args));
		}
		else if (current.type == TokenType::OpenBrace)
		{
			Block	*block = new Block(statement.value, args);
			_index++;
			while (!isEofToken() && _tokens[_index].type != TokenType::CloseBrace)
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

struct	LocationConfig
{
	std::string					path;
	std::string					root;
	std::string					index;
	bool						autoindex;
	std::vector<std::string>	allowed_methods;
	std::string					cgi_pass;
	size_t						client_max_body_size;
};

struct	ServerConfig
{
	std::string					host;
	int							port;
	std::vector<std::string>	server_name;
	std::string					root;
	std::vector<std::string>	index;
	std::map<int, std::string>	error_pages;
	std::vector<LocationConfig>	locations;

	ServerConfig() : host(""), port(-1), root("") {}
};


class	ConfigManager
{
private:
	std::vector<ServerConfig>	_servers;

	int		strToVal(const std::string& val)
	{
		if (val.empty())
			return (-1);
		char*	end;
		long	l = std::strtol(val.c_str(), &end, 10);
		if (*end != '\0' || l < 0)
			return (-1);
		return (static_cast<int>(l));
	}

	int		isValidPort(const std::string& port)
	{
		int		result = strToVal(port);
		if (result < 0 || result > 65535)
			throw std::runtime_error("Invalid port : " + port);
		return (result);
	}
	
	std::string	isValidIp(const std::string& ip)
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

	void	makeHost(const Directive* directive, ServerConfig& config)
	{
		if (directive->arguments.size() != 1 || config.port != -1)
			throw std::runtime_error("Host error or doublon listen");
		
		std::string	listenArg = directive->arguments[0];
		std::size_t	sep = listenArg.find(':');
		if (sep == std::string::npos)
		{
			config.port = isValidPort(listenArg);
		}
		else
		{
			config.host = isValidIp(listenArg.substr(0, sep));
			config.port = isValidPort(listenArg.substr(sep + 1));
		}
	}

	void	makeServerName(const Directive* directive, ServerConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() == 0)
			throw std::runtime_error("Missing arguments in server_name directive");
		if (config.server_name.size() != 0)
		{
			std::cout << "WARNING : server_name already populated earlier, overwriting with current server_name" << std::endl;
			config.server_name.clear();
		}
		for (size_t i = 0; i < args.size(); i++)
		{
			if (!args[i].empty())
				config.server_name.push_back(args[i]);
		}
	}
	
	void	makeServerRoot(const Directive* directive, ServerConfig& config)
	{
		std::vector<std::string>	args = directive->arguments;
		if (args.size() != 1 || args[0].empty())
			throw std::runtime_error("Missing or invalid argument in root directive");
		if (!config.root.empty())
			std::cout << "WARNING: root already defined earlier, overwriting with current directive" << std::endl;
		config.root = args[0];
	}

	void	makeIndex(const Directive* directive, ServerConfig& config)
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

	bool	isValidErrorUri(const std::string& uri)
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

	void	makeErrorPages(const Directive* directive, ServerConfig& config)
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
			if (config.error_pages.find(error) != config.error_pages.end())
				std::cout << "WARNING : Error " << error << " already mapped in error_pages, overwritten by current directive" << std::endl;
			config.error_pages[error] = args[last];
		}
	}

	void	parseServerDirective(const Directive* directive, ServerConfig& config)
	{
		if (directive->name == "listen")
			makeHost(directive, config);
		else if (directive->name == "server_name")
			makeServerName(directive, config);
		else if (directive->name == "root")
			makeServerRoot(directive, config);
		else if (directive->name == "index")
			makeIndex(directive, config);
		else if (directive->name == "error_page")
			makeErrorPages(directive, config);
		else
			throw std::runtime_error("Unrecognized directive '" + directive->name + "' in server block");
		std::cout << "Directive " + directive->name + " added to the server config" << std::endl;
	}
	
	void	parseLocationDirective(const Directive* directive, LocationConfig& config)
	{
		std::cout << "Directive " + directive->name + " added to the location config" << std::endl;
	}

	void	parseLocationBlockArgs(const Block* block, LocationConfig& locConfig)
	{
		// location / {
		// 	proxy_pass http://localhost:3000;
		// }

		// location /images/ {
		// 	root /data;
		// }

		// location = /favicon.ico {
		// 	log_not_found off;
		// }

		// location ~ \.php$ {
		// 	fastcgi_pass 127.0.0.1:9000;
		// }	
	}

	void	parseLocationBlock(const Block* block, ServerConfig& config)
	{
		LocationConfig	locConfig;
		
		parseLocationBlockArgs(block, locConfig);
		for (size_t i = 0; i < block->children.size(); i++)
		{
			IConfigNode*	child = block->children[i];
			std::cout << "Location block children n°" << i << " :\t";
			if (child->isBlock())
				throw std::runtime_error("Block found in location block");
			
			Directive*	childDirective = dynamic_cast<Directive*>(child);
			if (!childDirective)
				throw std::runtime_error("Dynamic cast childDirective failed");
			parseLocationDirective(childDirective, locConfig);
		}
		config.locations.push_back(locConfig);
		std::cout << "Location block added to the server config" << std::endl;
	}

	void	parseServerBlock(const Block* block)
	{
		ServerConfig	config;
		
		for (size_t i = 0; i < block->children.size(); i++)
		{
			IConfigNode*	child = block->children[i];
			std::cout << "Server block children n°" << i << " :\t";
			if (child->isBlock())
			{
				Block*	childBlock = dynamic_cast<Block*>(child);
				if (!childBlock)
					throw std::runtime_error("Dynamic cast childBlock failed");
				if (childBlock->name != "location")
					throw std::runtime_error("Unrecognized location block inside server");
				parseLocationBlock(childBlock, config);
				continue ;
			}
			Directive*	childDirective = dynamic_cast<Directive*>(child);
			if (!childDirective)
				throw std::runtime_error("Dynamic cast childDirectiveaaa failed");
			parseServerDirective(childDirective, config);
		}
		_servers.push_back(config);
		std::cout << "Server config added to the manager vector of servers" << std::endl;
	}


	void	validateServerConfig(const ServerConfig& config) {}
	void	validateLocationConfig(const LocationConfig& config) {}

public:
	ConfigManager(void) {}
	~ConfigManager(void) {}

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
				parseServerBlock(block);
				continue ;
			}
			throw std::runtime_error("Unrecognized server block at top ast level");
		}
	}
	

	const	std::vector<ServerConfig>& getServerConfigs(void) const { return _servers; }
	
};

int main(void)
{
	ConfigFile	file("lol.conf");
	Lexer		lexer(file.getContent());
	Parser		parser(lexer.getTokens());
	parser.showAst();
	std::cout << std::string(100, '*') << std::endl;

	ConfigManager	config;
	config.loadFromAst(parser.getAst());
	ServerConfig	serverConfig = config.getServerConfigs()[0];
	std::cout << serverConfig.host << " " << serverConfig.port << std::endl;
	std::cout << serverConfig.server_name[0] << " " << serverConfig.server_name[1] << std::endl;
	std::cout << serverConfig.root  << std::endl;
	std::cout << serverConfig.index[0] << " " << serverConfig.index[1] << std::endl;
	std::cout << serverConfig.error_pages[404] << std::endl;
	std::cout << serverConfig.error_pages[406] << std::endl;

    return (0);
}