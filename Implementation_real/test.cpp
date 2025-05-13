/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: qliso <qliso@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/05/12 15:41:23 by qliso             #+#    #+#             */
/*   Updated: 2025/05/12 16:21:06 by qliso            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include <string>
#include <iostream>
#include <climits>

size_t	strToValBytes(std::string const& val)
{
	char*	end;

	size_t	maxMb = 100U;
	size_t	maxKb = maxMb * 1024U;
	size_t	maxBytes = maxKb * 1024U;
	
	size_t	convert = strtoul(val.c_str(), &end, 10);
	
	if (*end == '\0' && convert <= maxBytes)
		return (convert);
	else if ((*end == 'k' || *end == 'K') && *(end + 1) == '\0' && convert <= maxKb)
		return (convert * 1024U);
	else if ((*end == 'm' || *end == 'M') && *(end + 1) == '\0' && convert <= maxMb)
		return (convert * 1024U * 1024U);
	return (ULONG_MAX);
}

void	test(const std::string& nb)
{
	std::cout << nb << "\t\t->\t" << strToValBytes(nb) << std::endl;
}

int main()
{
	test("salut");
	test("123456765434567654");
	test("104857600");
	test("104857599");
	test("104800");
	test("10M");
	test("50M");
	test("100M");
	test("101M");
	test("3Ma");
	test("5k");
	test("100k");
	test("5kz");
	return (0);
}