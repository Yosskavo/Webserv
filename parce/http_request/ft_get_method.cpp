#include "../../webserv.h"

void ft_get_method_line_info(std::string &s, t_string_split_http& splited_string)
{
	std::vector<std::string> v_str;

	v_str = ft_split(s, " ");
	if (v_str.size() >= 1)
		splited_string.method = v_str[0];
	else
	{
		splited_string.error = MISSING_METHOD;
		return ;
	}
	if (v_str.size() >= 2)
		splited_string.path = v_str[1];
	else
	{
		splited_string.error = MISSING_PATH;
		return ;
	}
	if (v_str.size() > 3)
	{
		splited_string.error = ALOT_OF_ARGUMENT_IN_METHOD_LINE;
		return ;
	}
	v_str = ft_split(v_str[2], "/");
	if (v_str.size() > 2)
	{
		splited_string.error = ALOT_OF_ARGUMENT_IN_PROTOCOLO_LINE;
	}
	if (v_str.size() >= 1)
	{
		splited_string.protocol = v_str[0];
	}
	else {
		splited_string.error = MISSING_PROTOCOL;
		return ;
	}
	if (v_str.size() == 2)
		splited_string.version = v_str[1];
	else
	 	splited_string.error = MISSING_VERIOSN;
}
