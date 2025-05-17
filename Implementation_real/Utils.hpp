/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Utils.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:29:17 by qliso             #+#    #+#             */
/*   Updated: 2025/05/17 23:29:45 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

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

std::string					fileToStr(const std::string& filename)
{
	std::ifstream	file(filename.c_str());
	std::ostringstream	buffer;
	
	if (!file)
		throw	std::runtime_error("Failed to open file: " + filename);
	buffer << file.rdbuf();
	file.close();
	return (buffer.str());
}

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

bool	strToBytes(std::string const& val, size_t& out, size_t maxBytes = 104857600U)
{
	const size_t 	maxKb = maxBytes / 1024U;
	const size_t	maxMb = maxKb / 1024U;
	char*			end;

	out = strtoul(val.c_str(), &end, 10);
	if (*end == '\0' && out <= maxBytes)
		return (true);
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
	std::vector<std::string>	segments;
	std::istringstream			iss(str);
	std::string					segment;
	
	while (std::getline(iss, segment, '/'))
	{
		if (segment != "." && !segment.empty())
			segments.push_back(segment);
	}
	
	str.clear();
	for (size_t i = 0; i < segments.size(); i++)
	str += "/" + segments[i];
	
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
		std::cout << "WARNING: " << folderPath << " does not exist yet" << std::endl;
		return (false);
	}
	
	if (!S_ISDIR(fileStatus.st_mode))
	{
		std::cout << "WARNING: " << folderPath << " exists but is not a directory" << std::endl;
		return (false);
	}
	
	if (access(folderPath.c_str(), X_OK | R_OK))
	{
		std::cout << "WARNING: " << folderPath << " cannot be accessed" << std::endl;
		return (false);
	}
	return (true);
}



#endif