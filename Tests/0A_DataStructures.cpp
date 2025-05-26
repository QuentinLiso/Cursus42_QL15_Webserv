/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0A_DataStructures.cpp                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/23 23:43:52 by qliso             #+#    #+#             */
/*   Updated: 2025/05/26 10:52:25 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "0A_DataStructures.hpp"


void HttpMethods::fillBiMap(BiMap<HttpMethods::Type>& bimap)
		{
				bimap.add("GET", HttpMethods::GET);
				bimap.add("POST", HttpMethods::POST);
				bimap.add("DELETE", HttpMethods::DELETE);
				bimap.add("PUT", HttpMethods::PUT);
		}

const BiMap<HttpMethods::Type>&	HttpMethods::map(void)
	{ 
		static BiMap<HttpMethods::Type> bimap;
		if (bimap.empty())
		fillBiMap(bimap);
		return (bimap);
	}

// =============================== DIRECTIVES

void Directives::fillBiMap(BiMap<Directives::Type>& bimap)
{
	bimap.add("listen", Directives::LISTEN);
	bimap.add("server_name", Directives::SERVER_NAME);
	
	bimap.add("alias", Directives::ALIAS);
	bimap.add("allowed_methods", Directives::ALLOWED_METHODS);
	bimap.add("cgi_pass", Directives::CGI_PASS);
	
	bimap.add("root", Directives::ROOT);
	bimap.add("index", Directives::INDEX);
	bimap.add("autoindex", Directives::AUTOINDEX);
	bimap.add("error_page", Directives::ERROR_PAGE);
	bimap.add("client_max_body_size", Directives::CLIENT_MAX_BODY_SIZE);
}


ParsingDirective*	Directives::createListen			(int line, int column, const TStr& name, const TStrVect& args) { return new Listen(line, column, name, args); }
ParsingDirective*	Directives::createServerName		(int line, int column, const TStr& name, const TStrVect& args) { return new ServerName(line, column, name, args); }
ParsingDirective*	Directives::createAlias				(int line, int column, const TStr& name, const TStrVect& args) { return new Alias(line, column, name, args); }
ParsingDirective*	Directives::createAllowedMethods	(int line, int column, const TStr& name, const TStrVect& args) { return new AllowedMethods(line, column, name, args); }
ParsingDirective*	Directives::createCgiPass			(int line, int column, const TStr& name, const TStrVect& args) { return new CgiPass(line, column, name, args); }
ParsingDirective*	Directives::createRoot				(int line, int column, const TStr& name, const TStrVect& args) { return new Root(line, column, name, args); }
ParsingDirective*	Directives::createIndex				(int line, int column, const TStr& name, const TStrVect& args) { return new Index(line, column, name, args); }
ParsingDirective*	Directives::createAutoindex			(int line, int column, const TStr& name, const TStrVect& args) { return new Autoindex(line, column, name, args); }
ParsingDirective*	Directives::createErrorPage			(int line, int column, const TStr& name, const TStrVect& args) { return new ErrorPage(line, column, name, args); }
ParsingDirective*	Directives::createClientMaxBodySize	(int line, int column, const TStr& name, const TStrVect& args) { return new ClientMaxBodySize(line, column, name, args); }

ParsingDirective*	Directives::createDirective(int line, int column, const TStr& name, const TStrVect& args, Directives::Type type)
{
	static std::map<Directives::Type, ParsingDirective* (*)(int, int, const TStr&, const TStrVect&) > functionsMap;
	if (functionsMap.empty())
	{
		functionsMap[Directives::LISTEN] = &createListen;
		functionsMap[Directives::SERVER_NAME] = &createServerName;
		functionsMap[Directives::ALIAS] = &createAlias;
		functionsMap[Directives::ALLOWED_METHODS] = &createAllowedMethods;
		functionsMap[Directives::CGI_PASS] = &createCgiPass;
		functionsMap[Directives::ROOT] = &createRoot;
		functionsMap[Directives::INDEX] = &createIndex;
		functionsMap[Directives::AUTOINDEX] = &createAutoindex;
		functionsMap[Directives::ERROR_PAGE] = &createErrorPage;
		functionsMap[Directives::CLIENT_MAX_BODY_SIZE] = &createClientMaxBodySize;
	}
	std::map<Directives::Type, ParsingDirective* (*)(int, int, const TStr&, const TStrVect&) >::const_iterator	it = functionsMap.find(type);
	if (it != functionsMap.end())
		return (it->second(line, column, name, args));
	return (NULL);
}

const BiMap<Directives::Type>&	Directives::map(void)
{ 
	static BiMap<Directives::Type> bimap;
	if (bimap.empty())
		fillBiMap(bimap);
	return (bimap);
}

bool	Directives::exists(const TStr& name, Directives::Type& type)
{
	bool	foundType = map().find(name, type);

	if (!foundType)
		type = Directives::UNKNOWN;
	return (foundType);
}


// ============================ BLOCKS

void Blocks::fillBiMap(BiMap<Blocks::Type>& bimap)
{
	bimap.add("server", Blocks::SERVER);
	bimap.add("location", Blocks::LOCATION);
}

const BiMap<Blocks::Type>&	Blocks::map(void)
{ 
	static BiMap<Blocks::Type> bimap;
	if (bimap.empty())
		fillBiMap(bimap);
	return (bimap);
}

bool	Blocks::exists(const TStr& name, Blocks::Type& type)
{
	bool	foundType = map().find(name, type);

	if (!foundType)
		type = Blocks::UNKNOWN;
	return (foundType);
}

ParsingBlock*	Blocks::createServer(int line, int column, const TStr& name, const TStrVect& args)
{ 
	return new ServerConfigBlock(line, column, name, args); 
}

ParsingBlock*	Blocks::createLocation(int line, int column, const TStr& name, const TStrVect& args)
{ 
	return new LocationConfigBlock(line, column, name, args); 
}

ParsingBlock*	Blocks::createBlock(int line, int column, const TStr& name, const TStrVect& args, Blocks::Type type)
{
	static std::map<Blocks::Type, ParsingBlock* (*)(int, int, const TStr&, const TStrVect&) > functionsMap;

	if (functionsMap.empty())
	{
		functionsMap[Blocks::SERVER] = &createServer;
		functionsMap[Blocks::LOCATION] = &createLocation;
	}
	std::map<Blocks::Type, ParsingBlock* (*)(int, int, const TStr&, const TStrVect&) >::const_iterator	it = functionsMap.find(type);
	if (it != functionsMap.end())
		return (it->second(line, column, name, args));
	return (NULL);
}