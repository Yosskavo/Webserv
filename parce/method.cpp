#include "../webserv.h"

void ft_methods_get(std::vector<std::string>::iterator & it, std::vector<std::string>::iterator & end, t_config & t)
{
	if (*it == ";")
		throw std::runtime_error("The allow_methods variable should have at least one allow method (POLL, GET or DELETE)");
	t.allow_method = 0;
	while (*it != ";")
	{
		if (*it == "GET")
			t.allow_method |= GET;
		else if (*it == "DELETE")
			t.allow_method |= DELETE;
		else if (*it == "POLL")
			t.allow_method |= POLL;
		else
		 	throw std::runtime_error("The mothod's gaving allowe_methods are invalide");
		it++;
		if (it == end)
			throw std::runtime_error("Every variable end with delimeter ';'");
	}
}
