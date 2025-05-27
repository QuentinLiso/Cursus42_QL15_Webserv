/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Includes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:21:21 by qliso             #+#    #+#             */
/*   Updated: 2025/05/27 18:26:35 by qliso            ###   ########.fr       */
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


namespace HttpMethods
{
	enum Type
	{
		UNKNOWN,
		GET,
		POST,
		DELETE,
		PUT
	};
};

class Token;
class Lexer;

class Statement;
class Parser;



#endif