#include "../webserv.h"

t_location::s_location(t_config &t)
{
	allow_method = t.allow_method;
	root_path = t.root_path;
	index = t.index;

}
