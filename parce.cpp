#include "webserv.h"
#include <csetjmp>
#include <cmath>

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

bool ft_check_is_number(std::string &it)
{
	size_t	i;

	i = 0;
	while (it[i])
	{
		if (!std::isdigit(it[0]))
			return (false);
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
	}
	return (j);
}

void ft_error_pages_get(std::vector< std::string >::iterator &it, std::vector<std::string>::iterator &end, t_config &t)
{
	std::vector<int>	v_error;
	std::string			s_error = "";
	size_t				tmp_error = 0;

	while (*it != ";")
	{
		if (!s_error.empty())
		{
			throw std::runtime_error("In error_page after the path it should have dilimeter");
		}
		if (ft_check_is_number(*it))
		{
			tmp_error = ft_to_number(*it);
			if (tmp_error < 300 || tmp_error >= 600)
				throw std::runtime_error("Invalide error code (the error code should be arround 300-599)");
			v_error.push_back(tmp_error);
		}
		else {
			s_error = *it;
		}
		it++;
		if (it == end)
			throw std::runtime_error("The variable should followed by a value and a dilimeter");
	}
	if (!v_error.size())
	{
		throw std::runtime_error("error_pages should contain a error numbers");
	}
	if (s_error.empty())
	{
		throw std::runtime_error("error_pages should have the path to the error display");
	}
	for (std::vector<int>::iterator iv = v_error.begin(); iv != v_error.end(); iv++)
	{
		t.error_pages[*iv] = s_error;
	}
}

void ft_methods_get(std::vector<std::string>::iterator & it, std::vector<std::string>::iterator & end, t_config & t)
{
	if (*it == ";")
		throw std::runtime_error("The allow_methods variable should have at least one allow method (POLL, GET or DELETE)");
	t.allow_method = 0;
	while (*it != ";")
	{
		if (*it == "GET")
			t.allow_method |= GET;
		else if (*it == "DELETE")
			t.allow_method |= DELETE;
		else if (*it == "POLL")
			t.allow_method |= POLL;
		else
		 	throw std::runtime_error("The mothod's gaving allowe_methods are invalide");
		it++;
		if (it == end)
			throw std::runtime_error("Every variable end with delimeter ';'");
	}
}

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

void	ft_handle_ather(std::vector<std::string>::iterator &it, std::vector<std::string>::iterator &end, t_config &t)
{
	if (*it == "listen")
	{
		it++;
		if (it == end || *it == ";")
		{
			throw std::runtime_error("Non value gaving for listen");
		}
		ft_port_ip_geter(*it, t);
		it++;
		if (end == it || *it != ";")
		{
			throw std::runtime_error("Every variable should end with delimeter ';'");
		}
	}
	else if (*it == "server_name")
	{
		it++;
		if (end == it || *it == ";")
		{
			throw std::runtime_error("Non value gaving for server_name");
		}
		while (*it != ";")
		{
			t.server_name.push_back(*it);
			it++;
			if (it == end)
				throw std::runtime_error("Every variable end with delimeter ';'");
		}
	}
	else if (*it == "client_max_body_size")
	{
		it++;
		if (it == end || *it == ";")
		{
			throw std::runtime_error("Non value gaving for client_max_body_size");
		}
		ft_get_max_body(*it, t);
		it++;
		if (end == it || *it != ";")
		{
			throw std::runtime_error("Every variable should end with delimeter ';'");
		}
	}
	else if (*it == "error_page")
	{
		it++;
		if (it == end || *it == ";")
		{
			throw std::runtime_error("Non value gaving for error_page");
		}
		ft_error_pages_get(it, end, t);
	
	}
	else if (*it == "root")
	{
		it++;
		if (it == end || *it == ";")
		{
			throw std::runtime_error("Non value gaving for root");
		}
		t.root_path = *it;
		it++;
		if (end == it || *it != ";")
		{
			throw std::runtime_error("Every variable should end with delimeter ';'");
		}
	}
	else if (*it == "index")
	{
		it++;
		if (it == end || *it == ";")
		{
			throw std::runtime_error("Non value gaving for index");
		}
		t.index = *it;
		it++;
		if (end == it || *it != ";")
		{
			throw std::runtime_error("Every variable should end with delimeter ';'");
		}
	}
	else if (*it == "allow_methods")
	{
		it++;
		if (it == end || *it == ";")
		{
			throw std::runtime_error("Non value gaving for allow_methods");
		}
		ft_methods_get(it, end, t);
	}
	else if (*it == "autoindex")
	{
		it++;
		if (it == end || *it == ";")
		{
			throw std::runtime_error("Non value gaving for index");
		}
		if (*it == "on" || *it == "ON")
			t.autoindex = true;
		else if (*it == "off" || *it == "OFF")
			t.autoindex = false;
		else
		 	throw std::runtime_error("In autoindex it should have eather on/off");
		it++;
		if (end == it || *it != ";")
		{
			throw std::runtime_error("Every variable should end with delimeter ';'");
		}
	}
	else {
		std::cerr << "The variable : " + *it << std::endl;
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
	for (std::vector<std::string>::iterator it = s_v.begin() + 2; it != s_v.end(); it++)
	{
		if (*it == "location")
		{
			// ft_handle_location(it, t);
		}
		else {
			std::vector<std::string>::iterator iv(  s_v.end() );
			ft_handle_ather(it, iv, t);
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
		v_config.push_back(ft_full_server_config(v_s));
		v_s.clear();
	}
	is.close();
	return (config);
}
