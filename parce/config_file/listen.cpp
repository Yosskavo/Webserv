#include "../../webserv.h"

static void ft_check_ip(std::string &s)
{
	int i, j, k, l;
	int res;

	res = std::sscanf(s.c_str(), "%d.%d.%d.%d", &i, &j, &k, &l);
	if (res != 4)
		throw std::runtime_error("Invalide formatte of ip it should be N.N.N.N");
	if (!(i >= 0 && i < 256) || !(i >= 0 && j < 256) || !(l >= 0 && l < 256) || !(k >= 0 && k < 256))
		throw std::runtime_error("The ip's bytes should be around 0-255");
}

void ft_port_ip_geter(std::string &it, t_config & t)
{
	size_t	i;
	char	*c;

	i = it.find(':');
	if (i != std::string::npos)
	{
		t.ip = it.substr(0, i);
		ft_check_ip(t.ip);
		t.port = std::strtod(it.substr(i + 1).c_str() , &c);
		if (*c != '\0')
			throw std::runtime_error("port should contain only numbers");
	}
	else {
		i = it.find('.');
		if (i != std::string::npos)
		{
			t.ip = it;
			ft_check_ip(t.ip);
		}
		else {
			t.port = std::strtod(it.c_str() , &c);
			if (*c != '\0')
				throw std::runtime_error("port should contain only numbers");
		}
	}
}
