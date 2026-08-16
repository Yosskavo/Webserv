#include "../../server.hpp"

static std::map<std::string, std::string> header_split(const std::string &tmp)
{
	std::vector<std::string> tmp_v;
	std::map<std::string, std::string> tmp_m;
	std::string						tmp_s;
	size_t				pos = 0;

	tmp_v = ft_split(tmp, "\r\n");
	for (std::vector<std::string>::iterator it = tmp_v.begin(); it != tmp_v.end(); it++)
	{
		pos = (*it).find(":");
		if (std::string::npos == pos)
			throw std::runtime_error("");
		tmp_s = ft_trim(it->substr(0, pos), ' ');
		if (tmp_s == "")
			throw std::runtime_error("");
		tmp_m[tmp_s] = ft_trim(it->substr(pos + 1), ' ');
	}
	return (tmp_m);
}

bool	ft_headers(t_request & req, std::string & s)
{
	std::map<std::string, std::string> tmp_map;
	std::string		tmp;
	size_t			pos;
	char			*c;

	pos = s.find("\r\n\r\n");
	if (std::string::npos == pos)
		return (false);
	tmp = s.substr(0, pos);
	pos += 4;
	s.erase(0, pos);
	try {
		tmp_map = header_split(tmp);
	}
	catch (const std::exception & e)
	{
		return (false);
	}
	if (tmp_map.end() != tmp_map.find("Host"))
	{
		req.server_name = tmp_map["Host"];
	}
	else
	 	return (false);
	if (tmp_map.end() != tmp_map.find("Content-Length"))
	{
		req.content_length = std::strtol(tmp_map["Content-Length"].c_str(), &c, 10);
	}
	if (tmp_map.end() != tmp_map.find("Cookie"))
	{
		req.cookies = ft_cookies(tmp_map["Cookie"]);
	}
	req.headers = tmp_map;
	return (true);
}
