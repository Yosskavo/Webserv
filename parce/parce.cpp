#include "../webserv.h"
#include <csetjmp>
#include <cmath>

// NOTE: skiping comment's and delete new line
// NOTE: Work on brakets {} and a functionality of nested brakest
// TODO: Work on accepting value

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
		if (*it == "}")
		{

		}
		else if (*it == "location")
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

void	ft_print_this(t_config & t)
{
	std::cout << "Ip : " << t.ip << std::endl;
	std::cout << "Port : " << t.port << std::endl;
	std::cout << "Root : " << t.root_path << std::endl;
	std::cout << "Allowed method : " << t.allow_method << std::endl;
	std::cout << "Client Max Body size : " << t.client_max_body_size << std::endl;
	std::cout << "Autoindex : " << t.autoindex << std::endl;
	std::cout << "Index : " << t.index << std::endl;
	std::cout << "Server Name : ";
	for (std::vector<std::string>::iterator it = t.server_name.begin(); it != t.server_name.end(); it++)
	{
		std::cout << *it << " ";
	}
	std::cout << std::endl;
	std::cout << "Error codes : " << std::endl;
	for (std::map<int, std::string>::iterator it = t.error_pages.begin(); it != t.error_pages.end(); it++)
	{
		std::cout << "\t--> " << it->first << " " + it->second << std::endl;
	}
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
		// int i =1;
		// for (std::vector<std::string>::iterator it = v_s.begin(); it != v_s.end(); it++)
		// {
		// 	std::cout << "line : " << i << " " << *it << std::endl;
		// 	i++;
		// }
		v_config.push_back(ft_full_server_config(v_s));
		ft_print_this(v_config[v_config.size() - 1]);
		v_s.clear();
	}
	is.close();
	return (config);
}
