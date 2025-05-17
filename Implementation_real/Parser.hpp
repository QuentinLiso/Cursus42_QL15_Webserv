/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:57:07 by qliso             #+#    #+#             */
/*   Updated: 2025/05/17 23:58:40 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include "Includes.hpp"
# include "DataStructures.hpp"
# include "Utils.hpp"

class	Parser
{
private:
	const std::vector<Token>&	_tokens;
	size_t						_index;
	std::vector<AConfigNode*>	_ast;

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

	AConfigNode*	buildSingleAst(void)
	{
		if (isEofToken() || _tokens[_index].type != TO_Identifier)
			throw std::runtime_error("Not an identifier token");
		
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
				throw std::runtime_error("Unclosed bracket");
			_index++;
			return (block);
		}
		throw std::runtime_error("Expected ';' or '{' after statement");
	}
	
	std::vector<AConfigNode*>	parseTokens(void)
	{
		std::vector<AConfigNode*>	ast;

		while (!isEofToken())
			ast.push_back(buildSingleAst());
		return (ast);
	}

	void	printNodes(const std::vector<AConfigNode*> node, size_t offset = 0) const
	{
		std::vector<AConfigNode*>::const_iterator	it = node.begin();
		std::vector<AConfigNode*>::const_iterator	ite = node.end();
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

	void	deleteNode(AConfigNode* node)
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

public:
	Parser(const std::vector<Token>& tokens)
		: _tokens(tokens), _index(0)
	{
		_ast = parseTokens();
	}
	
	virtual ~Parser(void)
	{
		for (std::vector<AConfigNode*>::iterator it = _ast.begin(); it != _ast.end(); it++)
			deleteNode(*it);
	}

	const std::vector<AConfigNode*>&	getAst(void) const { return _ast; }

	void	showAst(void) const
	{
		printNodes(_ast);
	}
};



#endif