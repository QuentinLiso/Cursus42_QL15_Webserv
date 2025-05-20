/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:29:17 by qliso             #+#    #+#             */
/*   Updated: 2025/05/19 21:54:09 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include "Includes.hpp"

// Some utilities
std::vector<std::string>    split(const std::string &str, const std::string& delimiters);

template < typename T >
bool	strToVal(const std::string& val, T& out)
{
	std::istringstream	iss(val);
	long long			result;
	
	iss >> result;
	
	if (iss.fail() || !iss.eof())
		return (false);
	if (!std::numeric_limits<T>::is_signed && result < 0)
		return (false);
	if (result < std::numeric_limits<T>::min() || result > std::numeric_limits<T>::max())
		return (false);
	
	out = static_cast<T>(result);
	return (true);
}

template < typename T>
std::string	convToStr(T& val)
{
	std::ostringstream	oss;
	oss << val;
	return (oss.str());
}

bool	strToBytes(std::string const& val, size_t& out, size_t maxBytes = 104857600U);


// Filepath utilities
std::string filepathSpecials(void);
bool		isValidFilepathChar(char c);
bool		isValidFilepath(const std::string& str);
void		removeStrTrailingChar(std::string& str, char c);

void		removeStrDuplicateChar(std::string& str, char c);
bool		containsDoubleDotsAccess(const std::string& str);
void		removeDotPaths(std::string& str);
void		normalizeFilepath(std::string& filepath);

std::string	joinPaths(const std::string& a, const std::string& b);
bool		isExecutableDirectory(const std::string& folderPath);
bool	isExistingFile(const std::string& filePath);
bool	isExistingAndAccessibleFile(const std::string& filePath, int accessArgs);


#endif