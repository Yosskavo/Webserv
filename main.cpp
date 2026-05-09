#include <iostream>

int main(int ac, char **av)
{
	if (ac != 2)
	{
		std::cerr << "Invalid number of argument (should be webserv [configfile])" << std::endl;
		return (1);
	}

    return 0;
}
