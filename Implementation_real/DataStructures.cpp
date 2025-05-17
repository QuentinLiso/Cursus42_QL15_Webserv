/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DataStructures.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 00:18:44 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "DataStructures.hpp"

Token::Token(TokenType t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c)
{}


AConfigNode::AConfigNode(const std::string& n, const std::vector<std::string>& args, int l, int c)
		: name(n), arguments(args), line(l), column(c)
{}

AConfigNode::~AConfigNode(void) {}


Directive::Directive(const std::string& n, const std::vector<std::string>& args, int l, int c)
        : AConfigNode(n, args, l, c) 
{}

bool    Directive::isBlock(void) const { return false; }


Block::Block(const std::string& n, const std::vector<std::string>& args, int l, int c) 
        : AConfigNode(n, args, l, c) 
{}
    
bool   Block::isBlock(void) const { return true; }
