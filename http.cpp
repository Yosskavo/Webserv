#include "tcp.h"

static void ft_parce_request(std::string &s, t_request &r)
{
	size_t		i = 0;
	size_t		j = 0;
	std::string		tmp;

	i = s.find(" ");
	if (i == std::string::npos)
	{
		throw std::runtime_error("method");
	}
	tmp = s.substr(0, i);
	if (tmp == "GET")
		r.method = GET;
	else if (tmp == "POST")
		r.method = POST;
	else if (tmp == "DELETE")
		r.method = DELETE;
	else
		throw std::runtime_error("method");
	i += 1;
	j = i;
	i = s.find(' ', i);
	if (i == std::string::npos)
	{
		throw std::runtime_error("path");
	}
	r.path = s.substr(j, i - j);
	if (r.path == "/")
		r.path = "/index.html";
	r.path = "html" + r.path;
	i += 1;
	j = i;
	i = s.find('/', i);
	if (i == std::string::npos)
	{
		throw std::runtime_error("protocol");
	}
	r.protocol = s.substr(j, i - j);
	if (i == std::string::npos)
	{
		throw std::runtime_error("protocol");
	}
	j = i += 1;
	i = s.length();
	char *c;
	r.version = std::strtod(s.substr(j, j - i).c_str(), &c) * 10;
}

t_request ft_parse_the_http_request(std::string & s)
{
	t_request				r;
	std::vector<std::string> v_s;
	std::string				tmp;
	std::vector<std::vector<std::string> > v_v_s;
	std::vector<std::string> v_s_e;
	size_t i = 0;
	bool flag = true;
	size_t j = 0;

	i = s.find("\r\n", 1);
	tmp = s.substr(0, i);
	ft_parce_request(tmp, r);
	s.erase(0, i + 2);
	i = 0;
	while (flag)
	{
		i = s.find("\r\n", i);
		if (i == std::string::npos)
		{
			flag = false;
			i = s.length();
		}
		tmp = s.substr(j, i - j);
		v_s_e.push_back(tmp);
		j = i += 2;
	}
	for (std::vector<std::string>::iterator it = v_s_e.begin(); it != v_s_e.end(); it++)
	{
		i = it->find(':');
		if (i == std::string::npos)
		{
			throw std::runtime_error("content");
		}
		r.content[it->substr(0, i)] = it->substr(i + 2);
	}
	return (r);
}
