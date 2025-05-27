/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:57:07 by qliso             #+#    #+#             */
/*   Updated: 2025/05/21 14:59:55 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Parser.hpp"

// PRIVATE
void			Parser::parseTokens(void)
{
	while (!isEofToken())
		_ast.push_back(buildSingleAst());
}

bool			Parser::isEofToken(void) const
{
	return (_index >= _tokens.size() || _tokens[_index].type == TO_EndOfFile);
}

AConfigNode*	Parser::buildSingleAst(void)
{
	const Token&				first = _tokens[_index];
	
	if (isEofToken() || first.type != TO_Identifier)
		_valid = Console::configLog(Console::ERROR, first.line, first.column, "PARSING", "Expected identifier token, found", first.value);
		
	std::vector<std::string>	args;
	Token						statement = setStatement(args);
	const Token&				current = _tokens[_index];

	if (current.type == TO_Semicolon)
	{
		_index++;
		return (new Directive(statement.value, args, statement.line, statement.column));
	}
	else if (current.type == TO_OpenBrace)
	{
		Block	*block = new Block(statement.value, args, statement.line, statement.column);
		_index++;
		while (!isEofToken() && _tokens[_index].type != TO_CloseBrace)
			block->children.push_back(buildSingleAst());
		if (isEofToken())
			_valid = Console::configLog(Console::ERROR, current.line, current.column, "PARSING", "No matching closing bracket for", current.value);
		_index++;
		return (block);
	}
	_valid = Console::configLog(Console::ERROR, current.line, current.column, "PARSING", "Expected ';' or '{' after statement", current.value);
	return (NULL);
}

Token			Parser::setStatement(std::vector<std::string>& args)
{
	Token	statement = _tokens[_index++];
	while (!isEofToken() && _tokens[_index].type != TO_Semicolon && _tokens[_index].type != TO_OpenBrace)
	{
		const Token&	current = _tokens[_index];
		if (current.type != TO_Identifier && current.type != TO_StringLiteral)
			_valid = Console::configLog(Console::ERROR, current.line, current.column, "PARSING", "Expected an identifier instead of ", current.value);
		args.push_back(current.value);
		_index++;
	}
	return (statement);
}


void			Parser::printNodes(const std::vector<AConfigNode*> node, size_t offset) const
{
	std::vector<AConfigNode*>::const_iterator	it = node.begin();
	std::vector<AConfigNode*>::const_iterator	ite = node.end();
	for (; it != ite; it++)
	{
		if ((*it)->isBlock())
		{
			const Block*	block = dynamic_cast<const Block*>(*it);
			if (!block)
			{
				Console::configLog(Console::ERROR, (*it)->line, (*it)->column, "PARSING", "Failed to cast AConfigNode* to Block*", (*it)->name);
				return ;
			}
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
			{
				Console::configLog(Console::ERROR, (*it)->line, (*it)->column, "PARSING", "Failed to cast AConfigNode* to Directive*", (*it)->name);
				return ;
			}
			std::cout << "\t" << std::string(offset * 3, '-') << "Directive : " << directive->name;
			for (size_t	i = 0; i < directive->arguments.size(); i++)
				std::cout << " " << directive->arguments[i];
			std::cout << std::endl;
		}
	}
}

void			Parser::deleteNode(AConfigNode* node)
{
	if (!node)
		return ;
	if (node->isBlock())
	{
		Block*	block = dynamic_cast<Block*>(node);
		if (block)
		{
			for (std::vector<AConfigNode*>::iterator it = block->children.begin(); it != block->children.end(); it++)
				deleteNode(*it);
		}
	}
	delete node;
}



// PUBLIC

Parser::Parser(const std::vector<Token>& tokens)
		: _tokens(tokens), _index(0), _valid(true)
{
	parseTokens();
}
	
Parser::~Parser(void)
{
	for (std::vector<AConfigNode*>::iterator it = _ast.begin(); it != _ast.end(); it++)
		deleteNode(*it);
}

const std::vector<AConfigNode*>&	Parser::getAst(void) const { return _ast; }

void			Parser::showAst(void) const
{
	if (_valid)
		printNodes(_ast);
}

void			Parser::checkValid(void) const
{
	if (_valid == false)
		throw std::runtime_error("Parsing failed");
}
