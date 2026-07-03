#include "../../webserv.h"

t_string_split_http ft_split_request(std::string &s)
{
	 t_string_split_http		splited_string;
	 std::string				str;
	 size_t						i;
	 std::vector<std::string>	v_str;

	 i = s.find("\r\n\r\n");
	 str = s.substr(0, i);
	 v_str = ft_split(str, "\r\n");
	 ft_get_method_line_info(v_str[0], splited_string);
	 for (std::vector<std::string>::iterator it = v_str.begin() + 1; it != v_str.end(); it++)
	 {
		 i = it->find(":");
		 splited_string.header[it->substr(0, i)] = it->substr(i);
	 }
	 return (splited_string);
}
