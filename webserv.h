#ifndef WEBSERV_H
# define WEBSERV_H

# include <iostream>
# include <fstream>
# include <string>
# include <cstdlib>
# include <sstream>
# include <cmath>
# include <vector>
# include <map>
# include <stack>
# include <exception>

# define MAX_GIGA 17179869183
# define MAX_MIGA 17592186044415
# define MAX_KILO 18014398509481983

typedef	enum e_method {
	GET = 1,
	POST = 2,
	DELETE = 4,
	UNKNOWN = 8
}	t_method;



typedef struct s_location {
	std::string path;
	short		allow_method;
	std::map<int, std::string>	return_path;
	std::string					root_path;
	std::vector<std::string>	index;
	bool						autoindex;
	bool						upload;
	std::string					upload_store;
	std::map<std::string, std::string>	cgi_ext;
	size_t						client_max_body_size;
	s_location(struct s_config &s);
}	t_location;

typedef struct s_config {
	std::vector<std::string>		server_name;
	std::map<int, std::string>	error_pages;
	size_t						client_max_body_size;
	size_t						port;
	std::string					ip;
	std::string					root_path;
	std::vector<std::string>					index;
	bool					autoindex;
	short						allow_method;
	std::vector<t_location>		location;

	s_config(void);
} t_config;

# include "templates/max_body.tpp"
# include "templates/method.tpp"

// NOTE: // *** Parcing *** //


void		ft_return_code_get(std::vector< std::string >::iterator &it, t_location &t);
void		ft_port_ip_geter(std::string &it, t_config & t);
void		ft_error_pages_get(std::vector< std::string >::iterator &it, std::vector<std::string>::iterator &end, t_config &t);
bool		ft_getserver(std::vector<std::string> &v_s, std::ifstream &is);
std::vector<t_config>	ft_parce_config(const char *path_to_config);
t_location		ft_handle_location(std::vector<std::string>::iterator &it, t_config &t);

// NOTE: this is for utils
void		ft_print_this(t_config & t);
int			ft_to_number(std::string &it);
bool		ft_check_is_number(std::string &it);

#endif
