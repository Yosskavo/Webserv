#include "webserv.h"
#include <csetjmp>

// NOTE: skiping comment's and delete new line
// NOTE: Work on brakets {} and a functionality of nested brakest
// TODO: Work on accepting value

bool	ft_is_white_spaces(char c)
{
	if (c == ' ' || (c >= 9 && 13 >= c))
	{
		return (true);
	}
	return (false);
}

bool	ft_getserver(std::vector<std::string> &v_s, std::ifstream &is)
{
	std::stack<char> sc;
	std::string	str = "";
	char		c;

	if (is.eof())
		return (false);
	while (is.get(c))
	{
		if (c == ';' || c == '{' || c == '}')
		{
			if (c == '{')
			{
				sc.push('{');
			}
			else if (c == '}')
			{
				if (sc.size())
					sc.pop();
				else
				 	throw std::runtime_error("a breaket that dont have peer '}'");
			}
			if (str.length())
				v_s.push_back(str);
			str = c;
			v_s.push_back(str);
			str = "";
		}
		else if (c == '#')
		{
			if (str.length())
			{
				v_s.push_back(str);
				str = "";
			}
			while (is.get(c))
			{
				if (c == '\n')
					break ;
			}
		}
		else if (ft_is_white_spaces(c) || c == '\n')
		{
			if (str.length())
			{
				v_s.push_back(str);
				str = "";
			}
		}
		else {
			str += c;
		}
	}
	if (sc.size())
	{
		throw std::runtime_error("A breaket doesn't have an end");
	}
	if (str.length())
		v_s.push_back(str);
	return (true);
}

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
		if (c != '\0')
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
			if (c != '\0')
				throw std::runtime_error("port should contain only numbers");
		}
	}
}

void	ft_handle_ather(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator &end, t_config &t)
{
	if (*it == "listen")
	{
		it++;
		if (*it == ";")
		{
			throw std::runtime_error("Non value gaving for listen");
		}
		ft_port_ip_geter(*it, t);
		if (*it != ";")
		{
			throw std::runtime_error("Every variable should end with delimeter ';'");
		}
	}
	else if (*it == "server_name")
	{
		it++;
		if (*it == ";")
		{
			throw std::runtime_error("Non value gaving for server_name");
		}
		while (*it != ";")
		{
			if (it == end)
				throw std::runtime_error("Every variable end with delimeter ';'");
			// WARN: this doesn't check if the actual server name is valid
			t.server_name.push_back(*it);
			it++;
		}
	}
	else if (*it == "client_max_body_size")
	{

	}
	else if (*it == "error_page")
	{
	}
	else {
		throw std::runtime_error("Invalide variable in the config file");
	}
}

t_config	ft_full_server_config(std::vector<std::string> &s_v)
{
	t_config t;

	if (s_v[0] != "server")
	{
		throw std::runtime_error("Unknown type ! it should start with server");
	}
	if (s_v[1] != "{")
	{
		throw std::runtime_error("The block diractive should start have a starting breaket");
	}
	for (std::vector<std::string>::iterator it = s_v.begin(); it != s_v.end(); it++)
	{
		if (*it == "location")
		{
			// ft_handle_location(it, t);
		}
		else {
			// ft_handle_ather(it, t);
		}
	}
	return (t);
}

t_config	ft_parce_config(const char *path_to_config)
{
	t_config		config;
	std::vector<t_config>		v_config;
	std::ifstream	is( path_to_config );
	// size_t				i = 1;
	std::vector<std::string> v_s;

	if (!is.is_open())
	{
		throw std::runtime_error(std::string("failed to open the file : ") + path_to_config);
	}
	while (ft_getserver(v_s, is))
	{
		int i =1;
		for (std::vector<std::string>::iterator it = v_s.begin(); it != v_s.end(); it++)
		{
			std::cout << "line : " << i << " " << *it << std::endl;
			i++;
		}
		// v_config.push_back(ft_full_server_config(v_s));
		v_s.clear();
	}
	is.close();
	return (config);
}
