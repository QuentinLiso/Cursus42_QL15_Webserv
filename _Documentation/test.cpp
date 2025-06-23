/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 10:16:39 by qliso             #+#    #+#             */
/*   Updated: 2025/06/20 15:48:44 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "srcs/Includes.hpp"

int main()
{
	int fd = open("/home/qliso/Documents/Webserv_github/steps.md", O_RDONLY);
	int fdkeep = fd;
	std::cout << fd << " " << fdkeep << std::endl;
	close(fd);
	std::cout << fd << " " << fdkeep << std::endl;


	return (0);
}

/*
Returns :
/
noslash
noslash
/
/
/
*/