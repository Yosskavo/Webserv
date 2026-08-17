#include "../../server.hpp"

bool	ft_cgi(std::string & out, const std::string & delim, t_response & res)
{
	std::vector<std::string> tmp_v;
	std::map<std::string, std::string> tmp_m;
	std::string							tmp;
	size_t			pos;
	char			*c;

	tmp_v = ft_split(out, delim);
	for (std::vector<std::string>::iterator it = tmp_v.begin(); it != tmp_v.end(); it++)
	{
		pos = it->find(":");
		if (pos == std::string::npos)
			return (false);
		tmp = ft_trim(it->substr(0, pos), ' ');
		if (tmp == "")
			return (false);
		tmp_m[tmp] = ft_trim(it->substr(pos + 1), ' ');
	}
	res.headers = tmp_m;
	if (res.headers.end() != res.headers.find("Status") )
	{
		if (res.headers["Status"] == "")
			return (true);
		pos = res.headers["Status"].find(" ");
		tmp = res.headers["Status"].substr(0, pos);
		res.status_code = std::strtol(tmp.c_str(), &c, 10);
		if (*c != '\0' || res.status_code < 0 || res.status_code > 599)
			return (false);
		if (pos == std::string::npos)
			res.status = server::reason_sentence(res.status_code);
		else
			res.status = ft_trim(res.headers["Status"].substr(pos), ' ');
	}
	return (true);
}
