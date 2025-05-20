/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DataStructures.cpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/19 11:30:16 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

# include "DataStructures.hpp"

Token::Token(TokenType t, const std::string& v, int l, int c) 
        : type(t), value(v), line(l), column(c)
{}