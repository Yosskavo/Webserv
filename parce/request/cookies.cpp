#include "../../server.hpp"

std::map<std::string, std::string> ft_cookies(const std::string  s)
{
	std::map<std::string, std::string> tmp_m;
	std::vector<std::string>			tmp_v;
	std::string							str;
	size_t			pos;

	tmp_v = ft_split(s, ";");
	for (std::vector<std::string>::iterator it = tmp_v.begin(); it != tmp_v.end(); it++)
	{
		pos = it->find("=");
		if (pos != std::string::npos)
		{
			str = ft_trim(it->substr(0, pos), ' ');
			if (str == "")
				continue ;
			tmp_m[str] = ft_trim(it->substr(pos + 1), ' ');
		}
	}
	return (tmp_m);
}
