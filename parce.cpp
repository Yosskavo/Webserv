#include "webserv.h"

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

void	ft_handle_ather(std::vector<std::string>::iterator &it, t_config &t)
{
	if (*it == "listen")
	{
	}
	else if (*it == "server_name")
	{
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

// void ft_handle_

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
		v_config.push_back(ft_full_server_config(v_s));
		v_s.clear();
	}
	is.close();
	return (config);
}
