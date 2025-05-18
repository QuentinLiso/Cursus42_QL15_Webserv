/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Parser.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:57:07 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 15:31:11 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSER_HPP
# define PARSER_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "DataStructures.hpp"
# include "Utils.hpp"

class	Parser
{
private:
	const std::vector<Token>&	_tokens;
	size_t						_index;
	std::vector<AConfigNode*>	_ast;
	bool						_valid;

	bool	isEofToken(void) const;
	Token	setStatement(std::vector<std::string>& args);
	AConfigNode*	buildSingleAst(void);
	void	parseTokens(void);
	void	printNodes(const std::vector<AConfigNode*> node, size_t offset = 0) const;
	void	deleteNode(AConfigNode* node);

public:
	Parser(const std::vector<Token>& tokens);
	virtual ~Parser(void);
	
	const std::vector<AConfigNode*>&	getAst(void) const;
	void	showAst(void) const;
	void	checkValid(void) const;
};


#endif