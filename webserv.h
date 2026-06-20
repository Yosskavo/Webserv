#ifndef WEBSERV_H
# define WEBSERV_H

# include <iostream>
# include <fstream>
# include <string>
# include <cstdlib>
# include <sstream>
# include <vector>
# include <map>
# include <stack>
# include <exception>

typedef	enum e_method {
	GET = 1,
	POLL = 2,
	DELETE = 4,
	UNKNOWN = 8
}	t_method;

typedef struct s_config {
	std::vector<std::string>		server_name;
	std::map<int, std::string>	error_pages;
	size_t						client_max_body_size;
	size_t						port;
	std::string					ip;
	std::string					root_path;
	std::string					index;
	bool					autoindex;
	short						allow_method;
} t_config;

t_config	ft_parce_config(const char *path_to_config);

#endif
