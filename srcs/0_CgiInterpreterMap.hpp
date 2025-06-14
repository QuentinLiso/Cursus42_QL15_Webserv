/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   0_CgiInterpreterMap.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/05 17:20:51 by qliso             #+#    #+#             */
/*   Updated: 2025/06/13 18:47:17 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGI_INTERPRETER_MAP_HPP
# define CGI_INTERPRETER_MAP_HPP

# include "Includes.hpp"


class CgiInterpreterMap
{
	private:
		static std::map<TStr, std::set<TStr> >	_map;
		static std::set<TStr>					_extensions;

		static std::map<TStr, std::set<TStr> >	createMap(void);
		static std::set<TStr>					createExtensions(void);
		
	public:
		static const std::map<TStr, std::set<TStr> >&	getMap(void);
		
		static bool	isValidPair(const TStr& interpreter, const TStr& extension);
		static bool	areValidPairs(const TStr& interpreter, const std::set<TStr>& extensions);
		static bool	isValidCgiInterpreter(const TStr& interpreter);
		static bool	isValidCgiExtension(const TStr& extension);

};


#endif


