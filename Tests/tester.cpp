#include <map>
#include <iostream>
#include <string>

int main()
{
	std::map<ushort, std::string>	map;

	std::cout << map.end()->second << std::endl;
	return 0;
}