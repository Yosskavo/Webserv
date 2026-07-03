#include "../../webserv.h"

t_location		ft_handle_location(std::vector<std::string>::iterator &it, t_config &t)
{
	t_location	l(t);

	if (*it == "{" || *it == ";")
	{
		throw std::runtime_error("location variable should have path then the begining of block");
	}
	l.path = *it;
	for (std::vector<t_location>::iterator ij = t.location.begin(); ij != t.location.end(); ij++)
	{
		if (l.path == ij->path)
		{
			throw std::runtime_error("In location decleration they are multy definition of " +l.path);
		}
	}
	it++;
	if (*it != "{")
	{
		throw std::runtime_error("location should have the bigining of block '{' after giving path");
	}
	it++;
	while (*it != "}")
	{
		if (*it == "allow_methods")
		{
			it++;
			ft_methods_get(it, l);
		}
		else if (*it == "root")
		{
			it++;
			if (*it == ";" || *it == "}")
				throw std::runtime_error("root should have a path then a dilimiter ';'");
			l.root_path = *it;
			it++;
			if (*it != ";" || *it == "}")
				throw std::runtime_error("root should have a dilimiter ';' after the path");
		}
		else if (*it == "cgi_ext")
		{
			it++;
			if (*it == "}" || *it == ";")
			{
				throw std::runtime_error("Non value gaving for cgi_ext");
			}
			std::string tmp = *it;
			l.cgi_ext[tmp] = *(++it);
			if (*it == ";" || *it == "}")
				throw std::runtime_error("cgi should have extention then path to the handler extention");
			it++;
			if (*it != ";")
				throw std::runtime_error("cgi should have a dilimiter ';'");
		}
		else if (*it == "upload_enable")
		{
			it++;
			if (*it == "}" || *it == ";")
			{
				throw std::runtime_error("Non value gaving for upload_enable");
			}
			if (*it == "on" || *it == "ON")
				l.upload = true;
			else if (*it == "off" || *it == "OFF")
				l.upload = false;
			else
				throw std::runtime_error("In upload_enable it should have eather on/off");
			it++;
			if (*it != ";")
			{
				throw std::runtime_error("Every variable should end with delimeter ';'");
			}
		}
		else if (*it == "upload_store")
		{
			it++;
			if (*it == ";" || *it == "}")
				throw std::runtime_error("upload_store should have a path then a dilimiter ';'");
			l.upload_store = *it;
			it++;
			if (*it != ";" || *it == "}")
				throw std::runtime_error("upload_store should have a dilimiter ';' after the path");
		}
		else if (*it == "autoindex")
		{
			it++;
			if (*it == "}" || *it == ";")
			{
				throw std::runtime_error("Non value gaving for autoindex");
			}
			if (*it == "on" || *it == "ON")
				l.autoindex = true;
			else if (*it == "off" || *it == "OFF")
				l.autoindex = false;
			else
				throw std::runtime_error("In autoindex it should have eather on/off");
			it++;
			if (*it != ";")
			{
				throw std::runtime_error("Every variable should end with delimeter ';'");
			}
		}
		else if (*it == "index")
		{
			it++;
			if (*it == "}" || *it == ";")
			{
				throw std::runtime_error("Non value gaving for index");
			}
			while (*it != ";")
			{
				l.index.push_back(*it);
				it++;
				if (*it == "}")
					throw std::runtime_error("index need a dilimiter at the end ';'");
			}
		}
		else if (*it == "return")
		{
			it++;
			if (*it == ";" || *it == "}")
				throw std::runtime_error("the return variable should have first return code and path them dilimiter");
			ft_return_code_get(it, l);
		}
		else if (*it == "client_max_body_size")
		{
			it++;
			if (*it == ";" || *it == "}")
				throw std::runtime_error("client_max_body_size should have a size then a dilimiter ';'");
			ft_get_max_body(*it, l);
			it++;
			if (*it != ";" || *it == "}")
				throw std::runtime_error("client_max_body_size should have a dilimiter ';' after the size");
		}
		else {
			throw std::runtime_error("this variable is invalide in location");
		}
		it++;
	}
	if (!l.index.size())
		l.index = t.index;
	return (l);
}
