#include "../webserv.h"

void ft_get_max_body(std::string &s, t_config &t)
{
	size_t	size;
	char	*c;
	short	p;

	size = std::strtoul(s.c_str(), &c, 10);
	if (*c == '\0')
		p = 0;
	else if (*c == 'K' || *c == 'k')
		p = 1;
	else if (*c == 'M' || *c == 'm')
		p = 2;
	else if  (*c == 'G' || *c == 'g')
		p = 3;
	else
		throw std::runtime_error("Invalide character in max body");
	//WARN: should handle the overflow
	t.client_max_body_size = size * std::pow(2, p * 10);
}
