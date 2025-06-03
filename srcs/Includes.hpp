/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Includes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:21:21 by qliso             #+#    #+#             */
/*   Updated: 2025/06/01 10:51:55 by qliso            ###   ########.fr       */
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
#include <iomanip>
#include <limits>

#include <cstdarg>
#include <cmath>
#include <climits>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

class Token;
class Lexer;

class Statement;
class Parser;
class ServerConfig;

typedef std::vector<std::string>	TStrVect;
typedef std::vector<int>			TIntVect;
typedef std::vector<ushort>			TUShortVect;
typedef std::string					TStr;
typedef std::pair<u_int32_t, u_int16_t>	TIPPort;
typedef	std::map<TStr, const ServerConfig*>	HostToServerMap;
typedef std::map<TIPPort, HostToServerMap>	RuntimeBuildMap;

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





#endif