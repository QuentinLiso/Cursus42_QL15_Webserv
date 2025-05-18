/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:54:57 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 15:52:03 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Lexer.hpp"

// PRIVATE
void					Lexer::fileToInput(const std::string& filename)
{
	std::ostringstream	buffer;
	std::ifstream	file(filename.c_str());
	
	if (!file)
	{
		Console::configLog(Console::ERROR, 0, 0, "LEXING", "Cannot open file", filename);
		_valid = false;
		return ;
	}	
	buffer << file.rdbuf();
	file.close();
	_input = buffer.str();
}

char	Lexer::peekNextChar(void) const
{
	if (_index >= _input.size()) 
		return ('\0');
	return (_input[_index]);
}

char	Lexer::advanceNextChar(void)
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

void	Lexer::skipWhitesAndComments(void)
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

bool	Lexer::isIdentifierChar(char c) const
{
	std::string	specials = "_./-$:@=~^*\\";
	return (std::isalnum(c) || specials.find(c) != std::string::npos);
}

std::string	Lexer::readIdentifier(void)
{
	std::string		str;
	while (isIdentifierChar(peekNextChar()))
		str += advanceNextChar();
	return (str);
}

std::string	Lexer::readQuotedString(char quote)
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

Token		Lexer::getNextToken(void)
{
	skipWhitesAndComments();
	char	c = peekNextChar();
	int		startLine = _line;
	int		startCol = _column;
	
	if (c == '\0')
		return (Token(TO_EndOfFile, "", startLine, startCol));
	else if (isIdentifierChar(c))
		return (Token(TO_Identifier, readIdentifier(), startLine, startCol));
	else if (c == '"' || c == '\'')
		return (Token(TO_StringLiteral, readQuotedString(c), startLine, startCol));
	else if (c == '{')
	{
		advanceNextChar();
		return (Token(TO_OpenBrace, "{", startLine, startCol));
	}
	else if (c == '}')
	{
		advanceNextChar();
		return (Token(TO_CloseBrace, "}", startLine, startCol));
	}
	else if (c == ';')
	{
		advanceNextChar();
		return (Token(TO_Semicolon, ";", startLine, startCol));
	}
	else
	{
		advanceNextChar();
		_valid = false;
		Console::configLog(Console::ERROR, _line, _column, "LEXING", "Unexpected character", std::string(1, c));
		return (Token(TO_Unknown, std::string(1, c), startLine, startCol));
	}
}

void	Lexer::tokenize(void)
{
	while (true)
	{
		Token toki = getNextToken();
		_tokens.push_back(toki);
		if (toki.type == TO_EndOfFile)
			break ;
	}
}


// PUBLIC
Lexer::Lexer(const std::string& filename)
	: _index(0), _line(1), _column(1), _valid(true)
	{
		fileToInput(filename);
		if (_input.empty())
			return ;
		tokenize();
	}

const std::vector<Token>&	Lexer::getTokens(void) const { return _tokens; }

void	Lexer::printTokens(void) const
{
	std::cout << "*** TOKENS : ***" << std::endl;
	for (size_t i = 0; i < _tokens.size(); i++)
		std::cout << _tokens[i].value << std::endl;
	std::cout << "*************" << std::endl;
}

void	Lexer::checkValid(void) const
{
	if (_valid == false)
		throw std::runtime_error("Lexing failed");
}