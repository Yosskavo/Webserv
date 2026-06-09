#ifndef TCP_H
# define TCP_H

# include <iostream>
# include <vector>
# include <map>
# include <string>
# include <cstring>
# include <cerrno>
# include <cstdio>
# include <fcntl.h>
# include <unistd.h>
# include <poll.h>
# include <netinet/in.h>
# include <sys/socket.h>

void			ft_bind(int fd, in_addr_t ip_addr, in_port_t port, sa_family_t family);
void			ft_listen(int fd, int max_connection);
void			ft_send(int fd, const char *str, size_t size, int flag);
void			ft_close(struct pollfd *fd, size_t &size);
int				ft_resv(int	fd, std::string &s, int flags);
int				ft_socket(int domain, int type, int protocol);
struct pollfd	*ft_accept(struct pollfd *fds, size_t &size);
struct pollfd	*ft_erase(size_t pos, struct pollfd *fds, size_t &size);


#endif
