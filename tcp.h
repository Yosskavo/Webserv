#ifndef TCP_H
# define TCP_H

# include <iostream>
# include <vector>
# include <map>
# include <string>
# include <sstream>
# include <cstring>
# include <cstdlib>
# include <cerrno>
# include <cstdio>
# include <fcntl.h>
# include <unistd.h>
# include <poll.h>
# include <netinet/in.h>
# include <sys/socket.h>

# define ASDASD "asdasddsa"

typedef enum e_method{
	GET,
	DELETE,
	POST
}	t_method;

typedef struct s_request {
	t_method							method;
	std::string							path;
	std::string							protocol;
	short								version;
	std::map<std::string, std::string>	content;
}	t_request;

void			ft_bind(int fd, in_addr_t ip_addr, in_port_t port, sa_family_t family);
void			ft_listen(int fd, int max_connection);
void			ft_send(int fd, const char *str, size_t size, int flag);
void			ft_close(struct pollfd *fd, size_t &size);
int				ft_resv(int	fd, std::string &s, int flags, int revensts);
int				ft_socket(int domain, int type, int protocol);
struct pollfd	*ft_accept(struct pollfd *fds, size_t &size);
struct pollfd	*ft_erase(size_t pos, struct pollfd *fds, size_t &size);
t_request			ft_parse_the_http_request(std::string & s);

#endif
