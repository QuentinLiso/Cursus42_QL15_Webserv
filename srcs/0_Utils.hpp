/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0_Utils.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:29:17 by qliso             #+#    #+#             */
/*   Updated: 2025/06/24 05:12:49 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_HPP
# define UTILS_HPP

# include "Includes.hpp"

// Some utilities
std::vector<std::string>    split(const std::string &str, const std::string& delimiters);

bool	strToBytes(std::string const& val, size_t& out, size_t maxBytes = 304857600U);
bool	fileToStr(const std::string& filename, TStr& out);
char	safeStrIndex(const TStr& str, size_t i);
void	toLowerStr(TStr& str);
TStr	createLowercaseStr(const TStr& str);
bool	areCaseInsensitiveEquals(const TStr& a, const TStr& b);
TStr	trimHeadAndTail(const TStr& str);
TStr	trimHeadAndTailSpecific(const TStr& str, const TStr& chars);
TStr	ipHostByteOrderToStr(u_int32_t ip);

std::ostream& operator<<(std::ostream& o, const TStrVect& vect);
int		debugClose(int fd);

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
std::string	getFileExtension(const std::string& filepath);


// HTTP METHODS MAP

namespace HttpMethods
{
	enum Type
	{
		UNKNOWN,
		GET,
		POST,
		DELETE,
		PUT,
		HEAD
	};

	TStr	toString(HttpMethods::Type	method);
};


namespace	FdType
{
	enum Type
	{
		FD_UNDEFINED,
		FD_EPOLL,
		FD_LISTENING_SOCKET,
		FD_CLIENT_CONNECTION,
		FD_CGI_PIPE,
		FD_SIGNAL_PIPE
	};
	
	TStr	toString(FdType::Type type);
};



# include "0_Utils.tpp"


class	HttpMethodsMap
{
	private:
		static void fillBiMap(BiMap<HttpMethods::Type>& bimap);

	public:
		static const BiMap<HttpMethods::Type>&	map(void);
};



#endif