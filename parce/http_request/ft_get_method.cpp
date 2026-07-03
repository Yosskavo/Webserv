#include "../../webserv.h"

void ft_get_method_line_info(std::string &s, t_string_split_http& splited_string)
{
	std::vector<std::string> v_str;

	splited_string.flag = true;
	v_str = ft_split(s, " ");
	if (v_str.size() >= 1)
		splited_string.method = v_str[0];
	if (v_str.size() >= 2)
		splited_string.path = v_str[1];
	if (v_str.size() > 3)
	{
		splited_string.flag = false;
	}
	if (v_str.size() >= 3)
	{
		v_str = ft_split(v_str[2], "/");
		if (v_str.size() >= 1)
			splited_string.protocol = v_str[0];
		if (v_str.size() >= 2)
			splited_string.version = v_str[1];
		if (splited_string.flag && v_str.size() > 2)
		{
			splited_string.flag = false;
		}
	}
}
