/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Lexer.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:54:57 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 15:50:32 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef LEXER_HPP
# define LEXER_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "DataStructures.hpp"
# include "Utils.hpp"

class	Lexer
{
private:
	std::string			_input;
	size_t				_index;
	int					_line;
	int					_column;
	std::vector<Token>	_tokens;
	bool				_valid;
	
	void		fileToInput(const std::string& filename);
	char		peekNextChar(void) const;
	char		advanceNextChar(void);
	void		skipWhitesAndComments(void);
	bool		isIdentifierChar(char c) const;
	std::string	readIdentifier(void);
	std::string	readQuotedString(char quote);
	Token		getNextToken(void);
	void		tokenize(void);

public:
	Lexer(const std::string& filename);
	const std::vector<Token>&	getTokens(void) const;
	void	printTokens(void) const;
	void	checkValid(void) const;
};	// Lexer



#endif