#include "../webserv.h"

static void	ft_check_method(std::string &method, t_config &server_config, t_http & http)
{
	if (method == "GET")
	{
		http.method = GET;
	}
	else if (method == "DELETE")
	{
		http.method = DELETE;
	}
	else if (method == "POST")
	{
		http.method = POST;
	}
	else
	{
		http.method = ERROR;
	}
	if (http.method != ERROR && !( http.method & server_config.allow_method ))
	{
	}
}

// file path + folder

void	ft_handle_method(std::string method_str, t_config &server_config)
{
	std::vector<std::string>	v_str;
	t_http						http;

	v_str = ft_split(method_str, " ");
	if (!v_str.size() || v_str.size() != 3)
	{
		return ;
	}

}
