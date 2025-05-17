/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/09 19:32:00 by qliso             #+#    #+#             */
/*   Updated: 2025/05/18 00:01:25 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Includes.hpp"
#include "Utils.hpp"
#include "IncludeClasses.hpp"

int main(void)
{
	Lexer		lexer("lol2.conf");
	Parser		parser(lexer.getTokens());
	parser.showAst();
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;

	ConfigBuilder	config(parser.getAst());
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;

	config.printConfig(config.getServers());
	std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;
	    
	config.printConfig("127.0.0.1", 80, "lil.com", "/hello/");
	// config.printConfig(config.findOtherConfigs("0.0.0.0", 81));
	// std::cout << "\n" << std::string(100, '*') << "\n" << std::endl;
	    
	// config.printConfig("127.0.0.2", 80, "zizou.com");

	return (0);
}