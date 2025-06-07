/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   1_Lexing.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:54:57 by qliso             #+#    #+#             */
/*   Updated: 2025/05/22 10:40:02 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
# define LEXER_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"


class Token
{
	public:	
		enum  Type
		{
			Identifier,
			StringLiteral,
			Semicolon,
			OpenBrace,
			CloseBrace,
			EndOfFile,
			Unknown
		};	// TokenType
	
	private:
		Token::Type   	_type;
		std::string 	_value;
		int         	_line;
		int         	_column;

	public:
	    Token(Token::Type t, const std::string& v, int l, int c);
		~Token(void);

		Token::Type	type(void) const;
		const TStr&	value(void) const;
		int			line(void) const;
		int			column(void) const;
		void		error(void) const;

};	// Token



class	Lexer
{
private:
	TStr				_input;
	size_t				_index;
	int					_line;
	int					_column;
	std::vector<Token>	_tokens;
	bool				_valid;
	
	bool		fileToInput(const std::string& filename);
	void		tokenize(void);
	Token		getNextToken(void);
	void		skipWhitesAndComments(void);
	void		moveIndexToNextChar(void);
	void		updateLineColumn(char c);
	bool		isIdentifierChar(char c) const;
	std::string	readIdentifier(void);
	std::string	readQuotedString(char quote);

public:
	Lexer(const std::string& filename);
	const std::vector<Token>&	getTokens(void) const;
	void	printTokens(void) const;
	void	throwInvalid(void) const;
	bool	valid(void) const;
};	// Lexer



#endif