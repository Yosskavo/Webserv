#include "../webserv.h"

std::vector<std::string> ft_split(std::string &str, std::string delimiter)
{
	std::vector<std::string>	v_str;
	bool						flag = true;
	size_t						i = 0;
	size_t						j = 0;

	while (flag)
	{
		i = str.find(delimiter, i);
		if (i == std::string::npos)
		{
			i = str.length();
			flag = 0;
		}
		v_str.push_back(str.substr(j, i - j));
		j = i + delimiter.length();
	}
	return (v_str);
}
