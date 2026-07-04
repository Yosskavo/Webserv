#include "../webserv.h"

t_string_split_http::s_string_split_http()
{
	version = "";
	method = "";
	path = "";
	protocol = "";
	error = NONE;
}
