/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   DataStructures.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 15:39:13 by qliso            ###   ########.fr       */
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

enum	HttpMethods
{
	HTTP_GET,
	HTTP_POST,
	HTTP_DELETE,
	HTTP_PUT
};

struct Token
{
    TokenType   type;
    std::string value;
    int         line;
    int         column;

    Token(TokenType t, const std::string& v, int l, int c);
};	// Token

struct AConfigNode
{
	std::string					name;
	std::vector<std::string>	arguments;
	int							line;
	int							column;

	AConfigNode(const std::string& n, const std::vector<std::string>& args, int l, int c);
    virtual			~AConfigNode(void);
    virtual bool	isBlock(void) const = 0;
};	// AConfigNode

struct Directive : public AConfigNode
{
    Directive(const std::string& n, const std::vector<std::string>& args, int l, int c);
    bool    isBlock(void) const;
};

struct Block : public AConfigNode
{
    std::vector<AConfigNode*>   children;

    Block(const std::string& n, const std::vector<std::string>& args, int l, int c);
    bool    isBlock(void) const;
};	// Block

template < typename T >
struct IsSetValue
{
	bool	isSet;
	T		value;

	IsSetValue(void);
	IsSetValue(const T& val);
	IsSetValue& operator=(const IsSetValue& other);
};


template < typename T >
IsSetValue<T>::IsSetValue(void) 
		: isSet(false), value() 
{}

template < typename T >
IsSetValue<T>::IsSetValue(const T& val)
		: isSet(true), value(val) 
{}

template < typename T >
IsSetValue<T>& IsSetValue<T>::operator=(const IsSetValue& other)
{
	if (this != &other)
	{
		isSet = other.isSet;
		value = other.value;
	}
	return (*this);
}

typedef IsSetValue<bool> 			AutoIndexSettings;
typedef IsSetValue<size_t> 			ClientMaxBodySize;
typedef IsSetValue<unsigned short>	Port;



#endif