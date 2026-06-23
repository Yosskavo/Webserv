#include "../webserv.h"

void	ft_print_this(t_config & t)
{
	std::cout << "Ip : " << t.ip << std::endl;
	std::cout << "Port : " << t.port << std::endl;
	std::cout << "Root : " << t.root_path << std::endl;
	std::cout << "Allowed method : " << t.allow_method << std::endl;
	std::cout << "Client Max Body size : " << t.client_max_body_size << std::endl;
	std::cout << "Autoindex : " << t.autoindex << std::endl;
	std::cout << "Server Name : ";
	for (std::vector<std::string>::iterator it = t.server_name.begin(); it != t.server_name.end(); it++)
	{
		std::cout << *it << " ";
	}
	std::cout << std::endl;
	std::cout << "Index : ";
	for (std::vector<std::string>::iterator it = t.index.begin(); it != t.index.end(); it++)
	{
		std::cout << *it << " ";
	}
	std::cout << std::endl;
	std::cout << "Error codes : " << std::endl;
	for (std::map<int, std::string>::iterator it = t.error_pages.begin(); it != t.error_pages.end(); it++)
	{
		std::cout << "\t==> " << it->first << " : " + it->second << std::endl;
	}
	std::cout << "Location : " << std::endl;
	for (std::vector<t_location>::iterator it = t.location.begin(); it != t.location.end(); it++)
	{
		std::cout << "\t***********************************" << std::endl;
		std::cout << "\tpath : " << it->path << std::endl;
		std::cout << "\tRoot : " << it->root_path << std::endl;
		std::cout << "\tAllowed method : " << it->allow_method << std::endl;
		std::cout << "\tClient Max Body size : " << it->client_max_body_size << std::endl;
		std::cout << "\tAutoindex : " << it->autoindex << std::endl;
		std::cout << "\tupload : " << it->upload << std::endl;
		std::cout << "\tupload store : " << it->upload_store << std::endl;
		std::cout << "\treturn codes : " << std::endl;
		for (std::map<int, std::string>::iterator ij = it->return_path.begin(); ij != it->return_path.end(); ij++)
		{
			std::cout << "\t\t==> " << ij->first << " -> " + ij->second << std::endl;
		}
		std::cout << "\tIndex : ";
		for (std::vector<std::string>::iterator ij = it->index.begin(); ij != it->index.end(); ij++)
		{
			std::cout << *ij << " ";
		}
		std::cout << std::endl;
		std::cout << "\tcgi ext : " << std::endl;
		for (std::map<std::string, std::string>::iterator ij = it->cgi_ext.begin(); ij != it->cgi_ext.end(); ij++)
		{
			std::cout << "\t\t--> " << ij->first << " : " + ij->second << std::endl;
		}
	}
}
