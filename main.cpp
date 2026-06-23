#include "webserv.h"

int main(int ac, char **av)
{
	std::vector< t_config >	config;
	short			exit_status = 0;

	if (ac != 2)
	{
		std::cerr << "Error : Invalide argument ! (it should be /webserv [configuration file])" << std::endl;
		return (1);
	}
	try
	{
		config = ft_parce_config(av[1]);
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << std::endl;
		exit_status = 1;
	}
	// for (std::vector<t_config>::iterator it = config.begin(); it != config.end(); it++)
	// {
	// 	std::cout << "-----------------------------------------" << std::endl;
	// 	ft_print_this(*it);
	// }
	return (exit_status);
}
