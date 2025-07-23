/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0_CgiInterpreterMap.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 17:26:18 by qliso             #+#    #+#             */
/*   Updated: 2025/06/24 16:49:03 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "0_CgiInterpreterMap.hpp"

std::map<TStr, std::set<TStr> >	CgiInterpreterMap::_map = CgiInterpreterMap::createMap();
std::set<TStr>				CgiInterpreterMap::_extensions = CgiInterpreterMap::createExtensions();

std::map<TStr, std::set<TStr> >	CgiInterpreterMap::createMap(void)
{
	std::map<TStr, std::set<TStr> >	map;
	map["/usr/bin/python3"].insert(".py");
	map["/usr/bin/python3"].insert(".cgi");
	map["/usr/bin/php"].insert(".php");
	map["/usr/bin/perl"].insert(".pl");
	map["/usr/bin/perl"].insert(".cgi");
	map["/home/qliso/Documents/ubuntu_cgi_tester"].insert(".bla");
	return (map);
}

std::set<TStr>	CgiInterpreterMap::createExtensions(void)
{
	std::set<TStr>	extensions;
	extensions.insert(".py");
	extensions.insert(".cgi");
	extensions.insert(".pl");
	extensions.insert(".bla");
	return (extensions);
}

const std::map<TStr, std::set<TStr> >& CgiInterpreterMap::getMap(void) { return _map; }

bool	CgiInterpreterMap::isValidPair(const TStr& interpreter, const TStr& extension)
{
	std::map<TStr, std::set<TStr> >::const_iterator it = _map.find(interpreter);
	if (it == _map.end())
		return (false);
	return (it->second.find(extension) != it->second.end());
}

bool	CgiInterpreterMap::areValidPairs(const TStr& interpreter, const std::set<TStr>& extensions)
{
	for (std::set<TStr>::const_iterator it = extensions.begin(); it != extensions.end(); it++)
	{
		if (!isValidPair(interpreter, *it))
			return (false);
	}
	return (true);
}

bool CgiInterpreterMap::isValidCgiInterpreter(const TStr& interpreter) { return _map.find(interpreter) != _map.end(); }

bool CgiInterpreterMap::isValidCgiExtension(const TStr& extension) { return _extensions.find(extension) != _extensions.end(); }

