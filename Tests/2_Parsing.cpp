/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   2_Parsing.cpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/27 18:33:07 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "2_Parsing.hpp"



// ==================== STATEMENT ==================


Statement::Statement(int line, int column, const TStr& name, const TStrVect& args, Statement::Type statementType)
	: _line(line), _column(column), _name(name), _args(args), _statementType(statementType), _error(0)
{}

Statement::~Statement(void) {
	for (std::vector<Statement*>::iterator it = _children.begin(); it != _children.end(); ++it)
		delete *it;
}

int					Statement::getLine(void) const		{ return _line; }
int					Statement::getColumn(void) const		{ return _column; }
const TStr&			Statement::getName(void) const		{ return _name; }
const TStrVect&		Statement::getArgs(void) const		{ return _args; }
Statement::Type 	Statement::getStatementType (void) const 	{ return _statementType; }
const std::vector<Statement*>&	Statement::getChildren(void) const { return _children; }
int					Statement::getError(void) const 		{ return _error; };
void				Statement::setError(int error) 		{ _error = error; };

void			Statement::addChild(Statement* child) { _children.push_back(child); }

int				Statement::error(const TStr& msg)
{
	Console::configLog(Console::ERROR, _line, _column, _name, _args, "PARSING", msg);
	setError(1);
	return (1);
}

void			Statement::warning(const TStr& msg)
{
	Console::configLog(Console::WARNING, _line, _column, _name, _args, "PARSING", msg);
}

std::ostream&		Statement::print(std::ostream& o, size_t indent = 0) const
{
	o << TStr(indent, '-');
	
	switch (_statementType)
	{
	case Statement::BLOCK:
		o << "Block :";
		break ;
	case Statement::DIRECTIVE:
		o << "Directive :";
		break ;
	default:
		o << "Unknown :";
		break;
	}
	
	o << getName() << " [" << getArgs() << "]" << std::endl;
	
	for (size_t i = 0; i < _children.size(); i++)
		_children[i]->print(o, indent + 3);
	return (o);
}




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
		Statement*	statement = buildStatement();
		if (statement)
			_statements.push_back(statement);
	}
}

Statement*	Parser::buildStatement(void)
{
	if (_index >= _tokens.size())
	{
		error("Out of bounds for some reason lol");
		return (NULL);
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
			return (new Statement(name.line(), name.column(), name.value(), args, Statement::DIRECTIVE));
		
		case Token::OpenBrace:
		{
			_index++;		// We skip the current token, token is now the next token after the {
			Statement*	statement = new Statement(name.line(), name.column(), name.value(), args, Statement::BLOCK);
			while (_index < _tokens.size() && _tokens[_index].type() != Token::EndOfFile && _tokens[_index].type() != Token::CloseBrace)
			{
				Statement*	child = buildStatement();
				if (child)
					statement->addChild(child);
			}
			if (_index >= _tokens.size() || _tokens[_index].type() == Token::EndOfFile)
				error(name.line(), name.column(), name.value(), args, "No matching '}' for block");
			_index++;
			return (statement);
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
	for (std::vector<Statement*>::iterator it = _statements.begin(); it != _statements.end(); it++)
		delete *it;
}

const std::vector<Statement*>&	Parser::getStatements(void) const { return _statements; }

void			Parser::printStatements(void) const
{
	for (size_t i = 0; i < _statements.size(); i++)
		_statements[i]->print(std::cout, 0);
}

int		Parser::getError(void) const { return _error; }
void	Parser::throwInvalid(void) const { if (_error) throw std::runtime_error("Parsing failed"); }

