#include "../webserv.h"

std::string ft_join_the_map(std::map<std::string, std::string> & map_m) {
	std::string str_j = "";
	std::map<std::string, std::string>::iterator it = map_m.begin();

	if (!( map_m.size() ))
		return "";
	while (1)
	{
		str_j += it->first + "=" + it->second;
		it++;
		if (it == map_m.end())
			break ;
		else {
			str_j += "; ";
		}
	}
	return (str_j);
}
