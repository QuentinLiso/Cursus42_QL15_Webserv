/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   2_Parsing.hpp                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:23:51 by qliso             #+#    #+#             */
/*   Updated: 2025/05/27 18:32:13 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	PARSING_HPP
# define PARSING_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"
# include "1_Lexing.hpp"


// ==================== STATEMENT =======================

class	Statement
{
	public:
		enum Type
		{
			BLOCK,
			DIRECTIVE,
			UNKNOWN
		};

	private:
		int							_line;
		int 						_column;
		TStr						_name;
		TStrVect 					_args;
		Statement::Type				_statementType;
		std::vector<Statement *> 	_children;
		int							_error;
		

	public:
		Statement(int line, int column, const TStr& name, const TStrVect& args, Statement::Type statementType);
		virtual ~Statement(void);

		int						getLine(void) const;
		int						getColumn(void) const;
		const TStr& 			getName(void) const;
		const TStrVect&			getArgs(void) const;
		Statement::Type 		getStatementType (void) const;
		const std::vector<Statement*>&	getChildren(void) const;
		int						getError(void) const;
		void					setError(int error);

		void					addChild(Statement* child);


		int						error(const TStr& msg);
		void					warning(const TStr& msg);

		std::ostream&	print(std::ostream& o, size_t indent) const;
};



// ==================== PARSER =======================

class	Parser
{
private:
	const std::vector<Token>&	_tokens;
	size_t						_index;
	std::vector<Statement*>		_statements;
	int							_error;

	void			error(int line, int column, const TStr& name, const TStrVect& args, const TStr& msg);
	void			error(const TStr& msg);
	void			parseTokens(void);
	Statement* 		buildStatement(void);


public:
	Parser(const std::vector<Token>& tokens);
	virtual ~Parser(void);
	
	const std::vector<Statement*>&	getStatements(void) const;
	void	printStatements(void) const;

	int		getError(void) const;
	void	throwInvalid(void) const;

};


#endif