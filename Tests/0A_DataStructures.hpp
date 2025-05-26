/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0A_DataStructures.hpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 23:41:54 by qliso             #+#    #+#             */
/*   Updated: 2025/05/26 11:40:47 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	DATASTRUCTURES_HPP
# define DATASTRUCTURES_HPP

# include "Includes.hpp"
# include "Console.hpp"
# include "0_Utils.hpp"

class	HttpMethods
{
	public:
		enum Type
		{
			UNKNOWN,
			GET,
			POST,
			DELETE,
			PUT
		};

	private:
		static void fillBiMap(BiMap<HttpMethods::Type>& bimap);

	public:
		static const BiMap<HttpMethods::Type>&	map(void);
};

class	Directives
{
	public:
		enum Type
		{
			UNKNOWN,
			LISTEN,				// Server specific
			SERVER_NAME,		// Server specific

			ALIAS,				// Location specific
			ALLOWED_METHODS,	// Location specific
			CGI_PASS,			// Location specific

			ROOT,
			INDEX,
			AUTOINDEX,
			ERROR_PAGE,
			CLIENT_MAX_BODY_SIZE,
		};

	private:
		static void fillBiMap(BiMap<Directives::Type>& bimap);
		static ParsingDirective*	createListen			(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingDirective*	createServerName		(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingDirective*	createAlias				(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingDirective*	createAllowedMethods	(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingDirective*	createCgiPass			(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingDirective*	createRoot				(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingDirective*	createIndex				(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingDirective*	createAutoindex			(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingDirective*	createErrorPage			(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingDirective*	createClientMaxBodySize	(int line, int column, const TStr& name, const TStrVect& args);


	public:
		static const 	BiMap<Directives::Type>&	map(void);
		static			ParsingDirective*			createDirective(int line, int column, const TStr& name, const TStrVect& args, Directives::Type type);
		static bool		exists(const TStr& name, Directives::Type& type);
		
};

class	Blocks
{
	public:
		enum Type
		{
			UNKNOWN,
			SERVER,
			LOCATION
		};

	private:
		static void fillBiMap(BiMap<Blocks::Type>& bimap);
		static ParsingBlock*	createServer	(int line, int column, const TStr& name, const TStrVect& args);
		static ParsingBlock*	createLocation	(int line, int column, const TStr& name, const TStrVect& args);

	public:
		static const 	BiMap<Blocks::Type>&	map(void);
		static 			ParsingBlock*			createBlock(int line, int column, const TStr& name, const TStrVect& args, Blocks::Type type);
		static bool		exists(const TStr& name, Blocks::Type& type);
};



#endif