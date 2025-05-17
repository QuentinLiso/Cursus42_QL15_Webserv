/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:54:57 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 00:20:55 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"

class	Lexer
{
private:
	std::string			_input;
	size_t				_index;
	int					_line;
	int					_column;
	std::vector<Token>	_tokens;
	
	char	peekNextChar(void) const
	{
		if (_index >= _input.size()) 
			return ('\0');
		return (_input[_index]);
	}

	char	advanceNextChar(void)
	{
		if (_index >= _input.size())
			return ('\0');
		char	c = _input[_index++];
		if (c == '\n')
		{
			_line++;
			_column = 1;
		}
		else
			_column++;
		return (c);
	}

	void	skipWhitesAndComments(void)
	{
		while (true)
		{
			char	c = peekNextChar();
			if (std::isspace(c))
				advanceNextChar();
			else if (c == '#')
			{
				while (peekNextChar() != '\n' && peekNextChar() != '\0')
					advanceNextChar();
			}
			else
				break ;
		}
	}
	
	bool	isIdentifierChar(char c) const
	{
		std::string	specials = "_./-$:@=~^*\\";
		return (std::isalnum(c) || specials.find(c) != std::string::npos);
	}

	std::string	readIdentifier(void)
	{
		std::string		str;
		while (isIdentifierChar(peekNextChar()))
			str += advanceNextChar();
		return (str);
	}
	
	std::string	readQuotedString(char quote)
	{
		std::string		str;
		advanceNextChar();
		while (peekNextChar() != quote && peekNextChar() != '\0')
			str += advanceNextChar();
		if (peekNextChar() == '\0')
			throw std::runtime_error("Unterminated string literal");
		advanceNextChar();
		return (str);
	}

	Token		getNextToken(void)
	{
		skipWhitesAndComments();
		char	c = peekNextChar();
		int		startLine = _line;
		int		startCol = _column;

		if (c == '\0')
			return (Token(TO_EndOfFile, "", startLine, startCol));
		
		if (isIdentifierChar(c))
		{
			return (Token(TO_Identifier, readIdentifier(), startLine, startCol));
		}
		if (c == '"' || c == '\'')
		{
			return (Token(TO_StringLiteral, readQuotedString(c), startLine, startCol));
		}
		switch(c)
		{
			case	'{'	:
				advanceNextChar();
				return (Token(TO_OpenBrace, "{", startLine, startCol));
			case	'}'	:
				advanceNextChar();
				return (Token(TO_CloseBrace, "}", startLine, startCol));
			case	';'	:
				advanceNextChar();
				return (Token(TO_Semicolon, ";", startLine, startCol));
			default		:
				std::ostringstream err;
				err << "Unexpected character '" << c << "' at line " << _line << ", column " << _column;
				throw	std::runtime_error(err.str());
		}
	}

	void	tokenize(void)
	{
		while (true)
		{
			Token toki = getNextToken();
			_tokens.push_back(toki);
			if (toki.type == TO_EndOfFile)
				break ;
		}
	}

public:
	Lexer(const std::string& filename)
		: _index(0), _line(1), _column(1)
	{
		_input = fileToStr(filename);
		tokenize();
	}

	const std::vector<Token>&	getTokens(void) const { return _tokens; }
};	// Lexer
