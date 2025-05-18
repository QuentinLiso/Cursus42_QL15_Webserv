/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 19:32:00 by qliso             #+#    #+#             */
/*   Updated: 2025/05/19 00:08:51 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Includes.hpp"
#include "Utils.hpp"
#include "IncludeClasses.hpp"

int main(int ac, char **av)
{
	if (ac != 2)
		return (1);

	Console::setConfigFileName(av[1]);
	
	Lexer		lexer(av[1]);
	lexer.checkValid();

	Parser		parser(lexer.getTokens());
	parser.checkValid();
	parser.showAst();
	
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;

	ConfigBuilder	config(parser.getAst());
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;

	config.checkValid();
	config.printConfig();
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;
	    
	config.printConfig("127.0.0.1", 80, "lil.com", "/hello/");
	config.printConfig(config.findOtherConfigs("0.0.0.0", 81));
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;
	    
	config.printConfig("127.0.0.2", 80, "zizou.com");

	return (0);
}