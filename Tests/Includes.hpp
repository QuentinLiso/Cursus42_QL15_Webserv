/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Includes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:21:21 by qliso             #+#    #+#             */
/*   Updated: 2025/05/26 11:42:34 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef	INCLUDES_HPP
# define INCLUDES_HPP

#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>

#include <cstdarg>
#include <cmath>
#include <climits>
#include <sys/stat.h>
#include <unistd.h>

typedef std::vector<std::string>	TStrVect;
typedef std::vector<int>			TIntVect;
typedef std::vector<ushort>			TUShortVect;
typedef std::string					TStr;

class HttpMethods;
class Directives;
class Blocks;

class Token;
class Lexer;

class Parser;
class AParsingNode;
class ParsingBlock;
class ParsingDirective;
class Listen;
class ServerName;
class Alias;
class AllowedMethods;
class CgiPass;
class Root;
class Index;
class Autoindex;
class ErrorPage;
class ClientMaxBodySize;

class AConfigBlock;
class LocationConfigBlock;
class ServerConfigBlock;


class Builder;

#endif