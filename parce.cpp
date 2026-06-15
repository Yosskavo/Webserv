#include "webserv.h"

t_config	ft_parce_config(const char *path_to_config)
{
	t_config		config = {};
	std::ifstream	is(path_to_config);
	std::string		str;

	if (!is.is_open())
	{
		throw std::runtime_error(std::string("failed to open the file : ") + str);
	}
	while (std::getline(is, str))
	{

	}
	return (config);
}
