/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0_Utils.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:29:17 by qliso             #+#    #+#             */
/*   Updated: 2025/05/30 19:55:30 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include "Includes.hpp"

// Some utilities
std::vector<std::string>    split(const std::string &str, const std::string& delimiters);

bool	strToBytes(std::string const& val, size_t& out, size_t maxBytes = 104857600U);
bool	fileToStr(const std::string& filename, TStr& out);
char	safeStrIndex(const TStr& str, size_t i);
void	toLowerStr(TStr& str);
TStr	ipHostByteOrderToStr(u_int32_t ip);

std::ostream& operator<<(std::ostream& o, const TStrVect& vect);


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


# include "0_Utils.tpp"

// HTTP METHODS MAP
class	HttpMethodsMap
{
	private:
		static void fillBiMap(BiMap<HttpMethods::Type>& bimap);

	public:
		static const BiMap<HttpMethods::Type>&	map(void);
};



#endif