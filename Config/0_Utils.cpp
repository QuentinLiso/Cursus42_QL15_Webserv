/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0_Utils.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:29:17 by qliso             #+#    #+#             */
/*   Updated: 2025/05/30 19:56:34 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "0_Utils.hpp"

// Some utilities
std::vector<std::string>    split(const std::string &str, const std::string& delimiters)
{
    std::vector<std::string>    tokens;
    size_t                      start = str.find_first_not_of(delimiters);
    size_t                      end;

    while (start != std::string::npos)
    {
        end = str.find_first_of(delimiters, start);
        std::string token = str.substr(start, end - start);
        tokens.push_back(token);
        start = str.find_first_not_of(delimiters, end);
    }
    return (tokens);
}

bool	strToBytes(std::string const& val, size_t& out, size_t maxBytes)
{
	const size_t 	maxKb = maxBytes / 1024U;
	const size_t	maxMb = maxKb / 1024U;
	char*			end;

	
	out = strtoul(val.c_str(), &end, 10);
	
	if (*end == '\0' && out <= maxBytes)
	{
		return (true);
	}
	else if ((*end == 'k' || *end == 'K') && *(end + 1) == '\0' && out <= maxKb)
	{
		out *= 1024U;
		return (true);
	}
	else if ((*end == 'm' || *end == 'M') && *(end + 1) == '\0' && out <= maxMb)
	{
		out *= (1024U * 1024U);
		return (true);
	}
	return (false);
}

bool	fileToStr(const std::string& filename, TStr& out)
{
	std::ostringstream	oss;
	std::ifstream	file(filename.c_str());

	if (!file)
		return (false);
	oss << file.rdbuf();
	file.close();
	out = oss.str();
	return (true);
}

char	safeStrIndex(const TStr& str, size_t i)
{
	if (i >= str.size())
		return ('\0');
	return (str[i]);
}

void	toLowerStr(TStr& str)
{
	for (TStr::iterator it = str.begin(); it != str.end(); it++)
		*it = std::tolower(static_cast<unsigned char>(*it));
}

std::ostream&	operator<<(std::ostream& o, const TStrVect& vect)
{
	if (vect.size() == 0)
		return (o);
	o << vect[0];
	for (size_t i = 1; i < vect.size(); i++)
		o << " " << vect[i];
	return (o);
}

TStr	ipHostByteOrderToStr(u_int32_t ip)
{
	std::ostringstream	oss;
	oss << ((ip >> 24) & 0xFF) << "."
		<< ((ip >> 16) & 0xFF) << "."
		<< ((ip >> 8) & 0xFF) << "."
		<< (ip & 0xFF);
	return (oss.str());
}

// Filepath utilities
std::string filepathSpecials(void) { return "/-_." ;}

bool		isValidFilepathChar(char c)
{
	return (std::isalnum(c) || filepathSpecials().find(c) != std::string::npos);
}

bool		isValidFilepath(const std::string& str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (!isValidFilepathChar(str[i]))
			return (false);
	}
	return (true);
}

void		removeStrTrailingChar(std::string& str, char c)
{
	if (str.empty())
		return ;
	while (str.size() > 1 && str[str.size() - 1] == c)
		str.erase(str.size() - 1);
}

void		removeStrDuplicateChar(std::string& str, char c)
{
	std::string doublon = std::string(2, c);
	size_t	i = str.find(doublon);
	
	if (i == std::string::npos)
		return ;
	std::cout << "WARNING: duplicate char '" + std::string(2, c) + "' found in '" + str + "'";
	while (str.size() > 1 && i != std::string::npos)
	{
		str.erase(i + 1, 1);
		i = str.find(doublon);
	}
	std::cout << " -> Normalized to '" + str + "'" << std::endl;
	return ;
}

bool		containsDoubleDotsAccess(const std::string& str)
{
	std::vector<std::string>	segments;
	std::istringstream			iss(str);
	std::string					segment;
	
	while (std::getline(iss, segment, '/'))
	{
		if (segment == "..")
		return (true); 
	}
	return (false);
}

void		removeDotPaths(std::string& str)
{
	if (str.empty() || str == "/")
		return ;

	std::vector<std::string>	segments;
	std::istringstream			iss(str);
	std::string					segment;

	bool	startingSlash = (str[0] == '/');
	bool	trailingSlash = (str[str.size() - 1] == '/');

	while (std::getline(iss, segment, '/'))
	{
		if (segment != "." && !segment.empty())
			segments.push_back(segment);
	}

	size_t	bytes = str.size();
	str.clear();
	str.reserve(bytes);
	
	if (startingSlash)
		str += "/";

	if (!segments.empty()) {
		str += segments[0];
		for (size_t i = 1; i < segments.size(); i++)
			str += "/" + segments[i];
	}

	if (trailingSlash && !str.empty() && str[str.size() - 1] != '/')
		str += "/";

	if (str.empty())
		str = "/";
}

void		normalizeFilepath(std::string& filepath)
{
	removeStrDuplicateChar(filepath, '/');
	removeStrTrailingChar(filepath, '/');
	removeDotPaths(filepath);
}
	
std::string	joinPaths(const std::string& a, const std::string& b)
{
	if(a.empty())
		return (b);
	if (b.empty())
		return (a);
	if (a[a.size() - 1] == '/' && b[0] == '/')
		return (a + b.substr(1));
	else if (a[a.size() - 1] != '/' && b[0] != '/')
		return (a + "/" + b);
	else
		return (a + b);
}

bool		isExecutableDirectory(const std::string& folderPath)
{
	struct stat	fileStatus;
	int	status = stat(folderPath.c_str(), &fileStatus);
	
	if (status != 0)
	{
		// std::cout << "WARNING: " << folderPath << " does not exist yet" << std::endl;
		return (false);
	}
	
	if (!S_ISDIR(fileStatus.st_mode))
	{
		// std::cout << "WARNING: " << folderPath << " exists but is not a directory" << std::endl;
		return (false);
	}
	
	if (access(folderPath.c_str(), X_OK | R_OK))
	{
		// std::cout << "WARNING: " << folderPath << " cannot be accessed" << std::endl;
		return (false);
	}
	return (true);
}


bool	isExistingFile(const std::string& filePath)
{
	struct stat	fileStatus;
	
	if (stat(filePath.c_str(), &fileStatus) != 0)
		return (false);
	
	if (!S_ISREG(fileStatus.st_mode))
		return (false);

	return (true);
}


bool	isExistingAndAccessibleFile(const std::string& filePath, int accessArgs)
{
	struct stat	fileStatus;
	
	if (stat(filePath.c_str(), &fileStatus) != 0)
		return (false);
	
	if (!S_ISREG(fileStatus.st_mode))
		return (false);
	
	if (access(filePath.c_str(), accessArgs))
		return (false);
	return (true);
}

// HTTP METHODS

void HttpMethodsMap::fillBiMap(BiMap<HttpMethods::Type>& bimap)
{
	bimap.add("GET", HttpMethods::GET);
	bimap.add("POST", HttpMethods::POST);
	bimap.add("DELETE", HttpMethods::DELETE);
	bimap.add("PUT", HttpMethods::PUT);
}

const BiMap<HttpMethods::Type>&	HttpMethodsMap::map(void)
{ 
	static BiMap<HttpMethods::Type> bimap;
	if (bimap.empty())
		fillBiMap(bimap);
	return (bimap);
}
