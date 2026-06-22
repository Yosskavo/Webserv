#include "../webserv.h"

t_location::s_location(t_config &t)
{
	allow_method = t.allow_method;
	root_path = t.root_path;
	index = t.index;
	autoindex = t.autoindex;
	client_max_body_size = t.client_max_body_size;
}

t_config::s_config(void)
{
	port = 8000;
	ip = "0.0.0.0";
	client_max_body_size = std::pow(2, 20);
	root_path = "./html";
	autoindex = false;
	allow_method = GET | DELETE | POST;
}
