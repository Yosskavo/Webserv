#include "../webserv.h"

t_config::s_config(void)
{
	port = 8000;
	ip = "0.0.0.0";
	client_max_body_size = std::pow(2, 20);
	root_path = "./html";
	autoindex = false;
	allow_method = GET | DELETE | POST;
}
