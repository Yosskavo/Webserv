#include "../webserv.h"

t_location::s_location(t_config &t)
{
	allow_method = t.allow_method;
	root_path = t.root_path;
	autoindex = t.autoindex;
	client_max_body_size = t.client_max_body_size;
	upload = false;
}

