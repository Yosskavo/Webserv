#include "../webserv.h"

void	ft_handle_http_request(std::string http_str, t_config &server_config, t_http & http_info)
{
	t_string_split_http request_str;

	request_str = ft_split_request(http_str);
	
}
