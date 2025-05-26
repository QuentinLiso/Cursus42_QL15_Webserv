/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   2_Parsing.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/26 11:39:02 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "2_Parsing.hpp"


// ==================== PARSER =======================

// PRIVATE

void	Parser::error(int line, int column, const TStr& name, const TStrVect& args, const TStr& msg)
{
	Console::configLog(Console::ERROR, line, column, name, args, "PARSING", msg);
	_error = 1;
}

void	Parser::error(const TStr& msg)
{
	Console::configLog(Console::ERROR, 0, 0, "", "PARSING", msg);
	_error = 1;
}

void			Parser::parseTokens(void)
{
	if (_tokens.size() == 0)
	{
		error("Empty list of tokens, nothing to parse lol");
		return ;
	}
	while (_index < _tokens.size() && _tokens[_index].type() != Token::EndOfFile)
	{
		AParsingNode*	node = buildNode();
		if (node)
			_ast.push_back(node);
	}
}

AParsingNode*	Parser::buildNode(void)
{
	if (_index >= _tokens.size())
	{
		error("Out of bounds for some reason lol");
		return (NULL);
		// return (new AParsingNode(0, 0, "", TStrVect(), AParsingNode::UNKNOWN));
	}
		
	const	Token	name = _tokens[_index];		// token[index] is 'server' for example
	TStrVect		args;
	_index++;
	if (name.type() != Token::Identifier)
	{
		error(name.line(), name.column(), name.value(), args, "Expected identifier token");
		return (NULL);
	}

	while (_index < _tokens.size() && _tokens[_index].type() != Token::Semicolon && _tokens[_index].type() != Token::OpenBrace)
	{
		const Token&	arg = _tokens[_index];
		args.push_back(arg.value());
		if (arg.type() != Token::Identifier && arg.type() != Token::StringLiteral)
			error(name.line(), name.column(), name.value(), args, "Expected an identifier or a string literal token in a statement");
		_index++;
	}

	// Now token is either EOF, ';', '{'
	// We have the token name (1st token), the args, and the current token EOF, ';' or '{'
	const	Token& current = _tokens[_index];
	switch (current.type())
	{
		case Token::Semicolon:
			_index++;			// We skip the current token, token is now the next token after the ;
			Directives::Type	directiveType;
			if (!Directives::exists(name.value(), directiveType))		// We log error if the directive name is not known
				error(name.line(), name.column(), name.value(), args, "Unknown directive name");
			
			return (Directives::createDirective(name.line(), name.column(), name.value(), args, directiveType));
		
		case Token::OpenBrace:
		{
			_index++;		// We skip the current token, token is now the next token after the {
			Blocks::Type	blockType;
			if (!Blocks::exists(name.value(), blockType))		// We log error if the block name is not known
				error(name.line(), name.column(), name.value(), args, "Unknown block name");
			
			ParsingBlock*	block = Blocks::createBlock(name.line(), name.column(), name.value(), args, blockType);
			while (_index < _tokens.size() && _tokens[_index].type() != Token::EndOfFile && _tokens[_index].type() != Token::CloseBrace)
			{
				AParsingNode*	node = buildNode();
				if (node)
					block->addChild(node);
			}
			if (_index >= _tokens.size() || _tokens[_index].type() == Token::EndOfFile)
				error(name.line(), name.column(), name.value(), args, "No matching '}' for block");
			_index++;
			return (block);
		}
		
		default:
			error(name.line(), name.column(), name.value(), args, "Expecting '{' or ';' after a statement");
			break;
	}
	return (NULL);
}



// PUBLIC

Parser::Parser(const std::vector<Token>& tokens)
		: _tokens(tokens), _index(0), _error(0)
{
	parseTokens();
}
	
Parser::~Parser(void)
{
	for (std::vector<AParsingNode*>::iterator it = _ast.begin(); it != _ast.end(); it++)
		delete *it;
}

const std::vector<AParsingNode*>&	Parser::getAst(void) const { return _ast; }

void			Parser::printNodes(void) const
{
	for (size_t i = 0; i < _ast.size(); i++)
		_ast[i]->print(std::cout, 0);
}

int		Parser::getError(void) const { return _error; }
void	Parser::throwInvalid(void) const { if (_error) throw std::runtime_error("Parsing failed"); }






// ==================== A PARSING NODE ==================


AParsingNode::AParsingNode(int line, int column, const TStr& name, const TStrVect& args, AParsingNode::Type nodeType)
	: _line(line), _column(column), _name(name), _args(args), _nodeType(nodeType), _error(0)
{}

AParsingNode::~AParsingNode(void) {}


const TStr&			AParsingNode::getName(void) const		{ return _name; }
const TStrVect&		AParsingNode::getArgs(void) const		{ return _args; }
AParsingNode::Type 	AParsingNode::getNodeType (void) const 	{ return _nodeType; }
int					AParsingNode::getError(void) const 		{ return _error; };
void				AParsingNode::setError(int error) 		{ _error = error; };

int				AParsingNode::error(const TStr& msg)
{
	Console::configLog(Console::ERROR, _line, _column, _args, "PARSING", msg);
	setError(1);
	return (1);
}

void			AParsingNode::warning(const TStr& msg)
{
	Console::configLog(Console::WARNING, _line, _column, _args, "PARSING", msg);
}

// ==================== BLOCKS =======================

ParsingBlock::ParsingBlock(int line, int column, const TStr& name, const TStrVect& args, Blocks::Type blockType) 
				: AParsingNode(line, column, name, args, AParsingNode::BLOCK), _blockType(blockType)
{}
		
ParsingBlock::~ParsingBlock(void)
{
	for (std::vector<AParsingNode*>::iterator it = _children.begin(); it != _children.end(); ++it)
		delete *it;
}


Blocks::Type		ParsingBlock::getBlockType(void) const { return _blockType; }


bool				ParsingBlock::isAllowedDirective(Directives::Type type)
{
	return (_allowedDirectives().find(type) != _allowedDirectives().end());
}

bool				ParsingBlock::isAllowedSubBlock(Blocks::Type type)
{
	return (_allowedSubBlocks().find(type) != _allowedSubBlocks().end());
}


void				ParsingBlock::addChild(AParsingNode* child)
{
	if (child != NULL)
		_children.push_back(child);
}

std::ostream&		ParsingBlock::print(std::ostream& o, size_t indent = 0) const
{
	
	o << TStr(indent, '-') << "Block : " << getName() << " [" << getArgs() << "]" << std::endl;
	
	for (size_t i = 0; i < _children.size(); i++)
		_children[i]->print(o, indent + 3);
	return (o);
}


// ================== SERVER BLOCK ========================

const std::set<Directives::Type>&	ServerConfigBlock::_allowedDirectives(void) const
{
	static std::set<Directives::Type>	allowedDirectives;
	if (allowedDirectives.empty())
	{
		allowedDirectives.insert(Directives::LISTEN);
		allowedDirectives.insert(Directives::SERVER_NAME);
		allowedDirectives.insert(Directives::ROOT);
		allowedDirectives.insert(Directives::INDEX);
		allowedDirectives.insert(Directives::AUTOINDEX);
		allowedDirectives.insert(Directives::ERROR_PAGE);
		allowedDirectives.insert(Directives::CLIENT_MAX_BODY_SIZE);
	}
	return (allowedDirectives);
}

const std::set<Blocks::Type>&	ServerConfigBlock::_allowedSubBlocks(void) const
{
	static std::set<Blocks::Type>	allowedSubBlocks;
	if (allowedSubBlocks.empty())
	{
		allowedSubBlocks.insert(Blocks::LOCATION);
	}
	return (allowedSubBlocks);
}

ServerConfigBlock::ServerConfigBlock(int line, int column, const TStr& name, const TStrVect& args)
	: ParsingBlock(line, column, name, args, Blocks::SERVER)
{
	
}

ServerConfigBlock::~ServerConfigBlock(void)
{
	
}

// ================ LOCATION BLOCK =================

const std::set<Directives::Type>&	LocationConfigBlock::_allowedDirectives(void) const
{
	static std::set<Directives::Type>	allowedDirectives;
	if (allowedDirectives.empty())
	{
		allowedDirectives.insert(Directives::ALIAS);
		allowedDirectives.insert(Directives::ALLOWED_METHODS);
		allowedDirectives.insert(Directives::CGI_PASS);
		allowedDirectives.insert(Directives::ROOT);
		allowedDirectives.insert(Directives::INDEX);
		allowedDirectives.insert(Directives::AUTOINDEX);
		allowedDirectives.insert(Directives::ERROR_PAGE);
		allowedDirectives.insert(Directives::CLIENT_MAX_BODY_SIZE);
	}
	return (allowedDirectives);
}

const std::set<Blocks::Type>&	LocationConfigBlock::_allowedSubBlocks(void) const
{
	static std::set<Blocks::Type>	allowedSubBlocks;
	return (allowedSubBlocks);
}

LocationConfigBlock::LocationConfigBlock(int line, int column, const TStr& name, const TStrVect& args)
	: ParsingBlock(line, column, name, args, Blocks::LOCATION)
{
	
}

LocationConfigBlock::~LocationConfigBlock(void)
{
	
}













// ==================== DIRECTIVES =======================

ParsingDirective::ParsingDirective(int line, int column, const TStr& name, const TStrVect& args, Directives::Type directiveType) 
				: AParsingNode(line, column, name, args, AParsingNode::DIRECTIVE), _directiveType(directiveType)
{}

ParsingDirective::~ParsingDirective(void) {}

Directives::Type 	ParsingDirective::getDirectiveType(void) const { return _directiveType; }

std::ostream&		ParsingDirective::print(std::ostream& o, size_t indent = 0) const
{
	o << TStr(indent, '-') << "Directive : " << getName() << " [" << getArgs() << "]" << std::endl;
	return (o);
}

