/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DataStructures.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/20 17:31:52 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	DATASTRUCTURES_HPP
# define DATASTRUCTURES_HPP

# include "Includes.hpp"

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

// enum	HttpMethods
// {
// 	HTTP_UNKNOWN,
// 	HTTP_GET,
// 	HTTP_POST,
// 	HTTP_DELETE,
// 	HTTP_PUT
// };

struct Token
{
    TokenType   type;
    std::string value;
    int         line;
    int         column;

    Token(TokenType t, const std::string& v, int l, int c);
};	// Token


#endif