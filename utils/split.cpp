#include "../server.hpp"

std::vector<std::string> ft_split(const std::string & str, const std::string &dilm)
{
	std::vector<std::string> v;
	size_t		pos_new = 0;
	size_t		pos_old;

	while (1)
	{
		pos_old = pos_new;
		pos_new = str.find(dilm, pos_old);
		if (pos_new == std::string::npos)
		{
			if (pos_old < str.length())
				v.push_back(str.substr(pos_old));
			break ;
		}
		v.push_back(str.substr(pos_old, pos_new - pos_old));
		pos_new += dilm.size();
	}

	return (v);
}
