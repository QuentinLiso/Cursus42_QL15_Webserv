/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   3_Config.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/22 08:56:11 by qliso             #+#    #+#             */
/*   Updated: 2025/05/25 22:57:06 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "3_Config.hpp"


// ================	ACONFIGBLOCK ========================


AConfigBlock::AConfigBlock(void)
{
	
}

AConfigBlock::~AConfigBlock(void)
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


// ======================= BUILDER =============================
// Go through AST 
//  If Block :
// 		> Check if Block.type allowed at top
//  		>> if yes loop over children
//   			>>> if ParsingBlock, check that it is an allowed sub block
//   				>>>> if yes, push_back sub block config
//   				>>>> if no, error unexpected block type
//   			>>> if directive, check map and call respective function with current directive
//  		 >> if no, error no server at top
// 	Else error, block expected as top node



// AParsingNode = ParsingBlock ou ParsingDirective

void	Builder::astSemanticAnalysis(const std::vector<AParsingNode*>& ast)
{

	for (size_t i = 0; i < ast.size(); i++)
	{
		if (ast[i]->getNodeType() != AParsingNode::BLOCK)
		{
			std::cout << "Error expected block at top level" << std::endl;
			continue ;
		}
		ast[i]->parsingToSemantic();
		// ast[i]->print(std::cout, 0);
		// ast[i]->parsingToSemantic()

		// ast[i]->parsingToSemantic() 
	}
}


Builder::Builder(const std::vector<AParsingNode*>& ast)
{
	astSemanticAnalysis(ast);
}

Builder::~Builder(void)
{
	
}






