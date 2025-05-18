/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/18 12:10:58 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 12:50:17 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>
#include <sstream>
#include <iostream>
#include <cstdarg>

template < typename T >
std::string	toStr(const T& x) { std::ostringstream oss; oss << x; return oss.str(); }


int main()
{
	
	return (0);
}

// FATAL: CONFIG LEXING: config.conf(lline:ccolumn):  Unrecognized character 'c'
// LogLevel, ConfigStep, filename, line, column, msg, explicit line