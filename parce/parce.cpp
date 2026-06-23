#include "../webserv.h"
#include <csetjmp>
#include <cmath>

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
		if (*it != ";")
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
		if (*it != ";")
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
		if (it == end || *it == "}" || *it == ";")
		{
			throw std::runtime_error("Non value gaving for root");
		}
		t.root_path = *it;
		it++;
		if (*it != ";")
		{
			throw std::runtime_error("Every variable should end with delimeter ';'");
		}
	}
	else if (*it == "index")
	{
		it++;
		if (it == end || *it == "}" || *it == ";")
		{
			throw std::runtime_error("Non value gaving for index");
		}
		while (*it != ";")
		{
			t.index.push_back(*it);
			it++;
			if (*it == "}")
				throw std::runtime_error("index need a dilimiter at the end ';'");
		}
	}
	else if (*it == "allow_methods")
	{
		it++;
		if (it == end || *it == ";")
		{
			throw std::runtime_error("Non value gaving for allow_methods");
		}
		ft_methods_get(it, t);
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
		if (*it != ";")
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
	std::vector<std::string>	v_sl;

	if (s_v[0] != "server")
	{
		throw std::runtime_error("Unknown type ! it should start with server");
	}
	if (s_v[1] != "{")
	{
		throw std::runtime_error("The block diractive should start have a starting breaket");
	}
	for (std::vector<std::string>::iterator it = s_v.begin() + 2; *it != "}"; it++)
	{
		if (*it == "location")
		{
			it++;
			if (*it == "{" || *it == ";")
				throw std::runtime_error("location should have a value then a start");
			v_sl.push_back(*it);
			it++;
			if (*it != "{" || *it == ";")
			{
				throw std::runtime_error("location should have a begining block '{'");
			}
			while (*it != "}")
			{
				v_sl.push_back(*it);
				it++;
			}
			v_sl.push_back(*it);
		}
		else {
			std::vector<std::string>::iterator iv(  s_v.end() );
			ft_handle_ather(it, iv, t);
		}
	}
	if (!(t.index.size()))
	{
		t.index.push_back("index.html");
	}
	for (std::vector<std::string>::iterator it = v_sl.begin(); it != v_sl.end(); it++)
	{
		t_location l = ft_handle_location(it, t);
		t.location.push_back(l);
	}
	return (t);
}


std::vector<t_config>	ft_parce_config(const char *path_to_config)
{
	std::vector<t_config>		v_config;
	std::ifstream	is( path_to_config );
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
	return (v_config);
}
