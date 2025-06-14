/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 10:16:39 by qliso             #+#    #+#             */
/*   Updated: 2025/06/14 11:33:13 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "srcs/Includes.hpp"


TStr			getParentDirectory(const TStr& resolvedPath)
{

	if (resolvedPath.empty() || resolvedPath.size() == 1)
		return ("/");

	bool trailingSlash = (resolvedPath[resolvedPath.size() - 1] == '/');
	size_t	pos = trailingSlash ? 
			resolvedPath.find_last_of('/', resolvedPath.size() - 2) :
			resolvedPath.find_last_of('/');

	return (pos == TStr::npos ? "/" : resolvedPath.substr(0, pos));
}

int main()
{
	std::cout << getParentDirectory("/") << std::endl;
	std::cout << getParentDirectory("noslash/lol") << std::endl;
	std::cout << getParentDirectory("noslash/lol/") << std::endl;
	std::cout << getParentDirectory("noslashatall") << std::endl;
	std::cout << getParentDirectory("a") << std::endl;
	std::cout << getParentDirectory("a/") << std::endl;

	return (0);
}

/*
Returns :
/
noslash
noslash
/
/
/
*/