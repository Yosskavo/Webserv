#include "../webserv.h"

bool ft_check_is_number(std::string &it)
{
	size_t	i;

	i = 0;
	while (it[i])
	{
		if (!std::isdigit(it[i]))
			return (false);
		i++;
	}
	return (true);
}

int	ft_to_number(std::string &it)
{
	size_t i = 0;
	size_t j = 0;

	while (it[i])
	{
		j = j * 10 + it[i] - '0';
		i++;
	}
	return (j);
}
