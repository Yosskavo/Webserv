#include "webserv.h"

// TODO: skiping comment's and delete new line
// TODO: Work on brakets {} and a functionality of nested brakest
// TODO: Work on accepting value

t_config	ft_parce_config(const char *path_to_config)
{
	t_config		config;
	// std::vector<t_config>		v_config;
	std::ifstream	is( path_to_config );
	// size_t				i = 1;

	if (!is.is_open())
	{
		throw std::runtime_error(std::string("failed to open the file : ") + path_to_config);
	}
	is.get();
	// while ()
	// {
	//
	// }
	is.close();
	return (config);
}
