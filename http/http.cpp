#include "../webserv.h"

/**
 * @brief aksjfhaskjfskajaksjfjkasdsjdfhkasdfjh
 *
 * @param http_str 
 * @param server_config 
 * @return 
 */
t_http ft_handle_http_request(std::string http_str, t_config &server_config)
{
	t_string_split_http request_str;
	t_http				http_info;

	request_str = ft_split_request(http_str);
	if (request_str.error != NONE)
	{
		// TODO: here need to create a function that return the path and the error number
		return (http_info);
	}
	// TODO: 1 -> check the method function
	// TODO: 2 -> check protocol and version
	// TODO: 3 -> check the header one by one
}
