/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   1_Lexing.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:54:57 by qliso             #+#    #+#             */
/*   Updated: 2025/05/22 10:04:39 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "1_Lexing.hpp"


// TOKENS

Token::Token(Token::Type t, const std::string& v, int l, int c) 
        : _type(t), _value(v), _line(l), _column(c)
{}

Token::~Token(void) {}

Token::Type	Token::type(void) const { return _type; }
const TStr&			Token::value(void) const { return _value; }
int			Token::line(void) const { return _line; }
int			Token::column(void) const { return _column; }


// PRIVATE
bool					Lexer::fileToInput(const std::string& filename)
{
	if (fileToStr(filename, _input) == false)
	{
		_valid = false;
		Console::configLog(Console::ERROR, 0, 0, filename, "LEXER", filename + " couldn't be read");
	}
	return (_valid);
}

void		Lexer::tokenize(void)
{
	while (safeStrIndex(_input, _index))
	{
		Token token = getNextToken();
		_tokens.push_back(token);
		if (token.type() == Token::EndOfFile)
			break ;
	}
}

Token		Lexer::getNextToken(void)
{
	skipWhitesAndComments();
	char	c = safeStrIndex(_input, _index);
	int		startLine = _line;
	int		startCol = _column;
	
	switch(c)
	{
		case '\0':
			return (Token(Token::EndOfFile, "", startLine, startCol));
		case '{':
			moveIndexToNextChar();
			return (Token(Token::OpenBrace, "{", startLine, startCol));
		case '}':
			moveIndexToNextChar();
			return (Token(Token::CloseBrace, "}", startLine, startCol));
		case ';':
			moveIndexToNextChar();
			return (Token(Token::Semicolon, ";", startLine, startCol));
		case '"': case '\'':
			moveIndexToNextChar();
			return (Token(Token::StringLiteral, readQuotedString(c), startLine, startCol));
		default:
			if (isIdentifierChar(c))
				return (Token(Token::Identifier, readIdentifier(), startLine, startCol));
			moveIndexToNextChar();
			_valid = false;
			Console::configLog(Console::ERROR, _line, _column, "LEXING", "Unexpected character", std::string(1, c));
			return (Token(Token::Unknown, std::string(1, c), startLine, startCol));
	}
}

void	Lexer::skipWhitesAndComments(void)
{
	char c;
	
	while (true)
	{
		c = safeStrIndex(_input, _index);
		if (std::isspace(c))
			moveIndexToNextChar();
		else if (c == '#')
		{
			while (safeStrIndex(_input, _index) != '\n' && safeStrIndex(_input, _index) != '\0')
				moveIndexToNextChar();
		}
		else
			break ;
	}
}

void	Lexer::moveIndexToNextChar(void)
{
	char c = safeStrIndex(_input, _index);
	_index++;
	updateLineColumn(c);
}

void	Lexer::updateLineColumn(char c)
{
	if (c == '\n')
	{
		_line++;
		_column = 1;
	}
	else
		_column++;
}

bool	Lexer::isIdentifierChar(char c) const
{
	std::string	specials = "_./-$:@=~^*\\";
	return (std::isalnum(c) || specials.find(c) != std::string::npos);
}

std::string	Lexer::readQuotedString(char quote)
{
	moveIndexToNextChar();

	size_t	start_index = _index;
	char c = safeStrIndex(_input, _index);
	
	while (c != quote && c != '\0')
	{
		moveIndexToNextChar();
		c = safeStrIndex(_input, _index);
	}

	TStr	str(_input.substr(start_index, _index - 1 - start_index));
	if (safeStrIndex(_input, _index) == '\0')
	{
		_valid = false;
		Console::configLog(Console::ERROR, _line, _column, str, "LEXING", "Unterminated quoted string literal");
		return (str);
	}
	moveIndexToNextChar();
	return (str);
}

std::string	Lexer::readIdentifier(void)
{
	size_t	start_index = _index;
	char c = safeStrIndex(_input, _index);
	
	while (isIdentifierChar(c))
	{
		moveIndexToNextChar();
		c = safeStrIndex(_input, _index);
	}
	return (TStr(_input.substr(start_index, _index - start_index)));
}


// PUBLIC
Lexer::Lexer(const std::string& filename)
	: _index(0), _line(1), _column(1), _valid(true)
	{
		if (!fileToInput(filename) || _input.empty())
		{
			Console::configLog(Console::ERROR, 0, 0, filename, "LEXER", "Input config is empty, cannot set an empty config up");
			return ;
		}
		tokenize();
	}

const std::vector<Token>&	Lexer::getTokens(void) const { return _tokens; }

void	Lexer::printTokens(void) const
{
	std::cout << "*** TOKENS : ***" << std::endl;
	for (size_t i = 0; i < _tokens.size(); i++)
		std::cout << "Token : " << _tokens[i].value() << std::endl;
	std::cout << "*************" << std::endl;
}

void	Lexer::throwInvalid(void) const
{
	if (_valid == false)
		throw std::runtime_error("Lexing failed");
}

bool	Lexer::valid(void) const { return _valid; }