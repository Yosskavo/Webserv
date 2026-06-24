#include "../webserv.h"

t_http	ft_handle_http_request(std::string http_str, t_config &server_config)
{
	std::vector<std::string> v_str;
	std::string					tmp;
	t_http					http;
	size_t	i = 0;


	i = http_str.find("\r\n\r\n");
	if (i == std::string::npos)
	{
		http.method = ERROR;
		return (http);
	}
	tmp = http_str.substr(0, i);
	http_str.erase(0, i + 4);
	v_str = ft_split(http_str, "\r\n");
	ft_check_method(v_str[0], server_config);
	return (http);
}
