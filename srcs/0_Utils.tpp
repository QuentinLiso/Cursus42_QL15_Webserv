/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0_Utils.tpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/17 23:29:17 by qliso             #+#    #+#             */
/*   Updated: 2025/06/07 11:36:36 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef UTILS_TPP
# define UTILS_TPP

# include "Includes.hpp"


// Some utilities
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
std::string	convToStr(T val)
{
	std::ostringstream	oss;
	oss << val;
	return (oss.str());
}

template < typename T >
typename std::vector<T>::const_iterator&	safeVectIndex(const std::vector<T>& vect, size_t i)
{
	if (i >= vect.size())
		return (vect.end());
	return (vect.begin() + i);
}


template < typename T>
class	BiMap
{
	private:
		std::map<TStr, T>	_strToTypeMap;
		std::map<T, TStr>	_typeToStrMap;


	public:
		BiMap(void) {}
		~BiMap(void) {}

		BiMap&	add(const TStr& str, const T& type)
		{
			_strToTypeMap[str] = type;
			_typeToStrMap[type] = str;
			return (*this);
		}

		bool	find(const TStr& str) const
		{
			typename std::map<TStr, T>::const_iterator it = _strToTypeMap.find(str);
			if (it != _strToTypeMap.end())
				return (true);
			return (false);
		}
		
		bool	find(const TStr& str, T& _type) const
		{
			typename std::map<TStr, T>::const_iterator it = _strToTypeMap.find(str);
			if (it != _strToTypeMap.end())
			{
				_type = it->second;
				return (true);
			}
			return (false);
		}

		bool	find(const T& type) const
		{
			typename std::map<T, TStr>::const_iterator it = _typeToStrMap.find(type);
			if (it != _typeToStrMap.end())
				return (true);
			return (false);
		}

		bool	find(const T& type, TStr& _str) const
		{
			typename std::map<T, TStr>::const_iterator it = _typeToStrMap.find(type);
			if (it != _typeToStrMap.end())
			{
				_str = it->second;
				return (true);
			}
			return (false);
		}

		size_t	size(void) const { return (_strToTypeMap.size()); }
		bool	empty(void) const { return (_strToTypeMap.empty()); }
};




#endif