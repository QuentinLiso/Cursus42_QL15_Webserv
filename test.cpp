/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/06/10 10:16:39 by qliso             #+#    #+#             */
/*   Updated: 2025/06/12 17:18:28 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "srcs/Includes.hpp"




int main()
{
	for (unsigned int i = 0; i < 256; i++)
		if (isForbiddenRawByteUriPath(i))
			std::cout << "Lol" << std::endl;
	return (0);
}