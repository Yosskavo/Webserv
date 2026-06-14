#include "tcp.h"

int	ft_socket(int domain, int type, int protocol)
{
	int fd;
	socklen_t opt = 1;

	fd = socket(domain, type, protocol);
	if (fd < 0)
	{
		throw std::runtime_error("Socket failed");
	}
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(fd);
		throw std::runtime_error("SetSocketOpt failed");
	}
	return (fd);
}

void	ft_bind(int fd, in_addr_t ip_addr, in_port_t port, sa_family_t family)
{
	struct sockaddr_in s;

	std::memset(&s, 0, sizeof(s));
	s.sin_addr.s_addr = ip_addr;
	s.sin_port = htons(port);
	s.sin_family = family;
	if (bind(fd, (struct sockaddr *)&s, sizeof(s)) == -1)
	{
		throw std::runtime_error("Bind failed");
	}
}

void	ft_listen(int fd, int max_connection)
{
	if (listen(fd, max_connection))
	{
		throw std::runtime_error("Listen failed");
	}
}

struct pollfd	*ft_accept(struct pollfd *fds, size_t &size)
{
	std::vector<int>	fd;
	int					tmp;
	int					flag;
	struct sockaddr_in	s;
	socklen_t			len = sizeof(s);
	struct pollfd		*pfd;

	// while (true)
	// {
		std::memset(&s, 0, sizeof(s));
		tmp = accept(fds[0].fd, (struct sockaddr *)&s, &len);
		if (tmp == -1)
		{
			// break ;
		}
		std::cout << "A new Client : " << tmp << std::endl;
		flag = fcntl(tmp, F_GETFL, 0);
		fcntl(tmp, F_SETFL, flag | O_NONBLOCK);
		fd.push_back(tmp);
	// }
	pfd = new struct pollfd[size + fd.size()];
	for (size_t i = 0; i < size; i++)
	{
		pfd[i].fd = fds[i].fd;
		pfd[i].events = fds[i].events;
		pfd[i].revents = fds[i].revents;
	}
	delete [] fds;
	for (size_t i = 0; i < fd.size(); i++)
	{
		pfd[i + size].fd = fd[i];
		pfd[i + size].events = POLLIN | POLLOUT;
		pfd[i + size].revents = 0;
	}
	size = size + fd.size();
	return (pfd);
}

int		ft_resv(int	fd, std::string &s, int flags, int revents)
{
	int i = 0;
	char c[1025];
	size_t	pos;
	t_request	r;
	int f = 1;

	std::string	tmp;

	// TODO: this should be read all the data instead of just some
	i = recv(fd, &c, 1023, flags);
	if (i == -1)
	{
		throw std::runtime_error("Recv failed");
		return (0);
	}
	if (i == 0)
		return (0);
	c[i] = '\0';
	s += c;
	while (true)
	{
		i = recv(fd, &c, 1023, flags);
		if (i == -1 && errno == EAGAIN)
			break ;
		if (i == -1)
		{
			std::perror("Recv");
			throw std::runtime_error("Recv failed");
			return (0);
		}
		if (!i)
			break ;
		c[i] = '\0';
		s += c;
		if (i < 1023)
			break ;
	}
	pos = s.find("\r\n\r\n");
	if (pos == std::string::npos)
	{
		std::cout << "--- the client " << fd << " didn't complate his data --- \n" << std::endl;
		f = 2;
	}
	else {
		tmp = s.substr(0, pos + 4);
		std::cout << "--- the full data from client " << fd << " --- \n" << "The Http \n" << tmp << std::endl;
		std::cout << "size = " << tmp.length() << std::endl;
		s.erase(pos, pos + 4);
		if (s.find("\r\n\r\n") == std::string::npos)
		{
			std::cout << "This worked" << std::endl;
		}
		else {
			std::cout << "Didn't work" << std::endl;
		}
		r = ft_parse_the_http_request(s);
		s.erase(0, pos);
	}
	std::cout << "==> method   : " << r.method << std::endl;
	std::cout << "==> path     : " << r.path << std::endl;
	std::cout << "==> protocol : " << r.protocol << std::endl;
	std::cout << "==> version  : " << r.version / 10.0 << std::endl;
	for (std::map<std::string, std::string>::iterator it = r.content.begin(); it != r.content.end(); it++)
	{
		std::cout << "|-> " << it->first << " = " << it->second << std::endl;
	}
	if (revents & POLLOUT)
	{
		if (!f)
		{
			// std::string s = r.protocol + ;
		}
		else {
			
		}
	}
	return (f);
}

struct pollfd *ft_erase(size_t pos, struct pollfd *fd, size_t &size)
{
	std::cout << "pos : " <<  pos << "; fd : " << fd[pos].fd << "; size : " << size << std::endl;
	int j = 0;
	if (pos > size)
		return (fd);
	close(fd[pos].fd);
	struct pollfd *tmp;
	tmp = fd + pos;
	tmp = new struct pollfd[size - 1];
	for (size_t i = 0; i < size; i++)
	{
		if (i == pos)
			continue ;
		tmp[j].fd = fd[i].fd;
		tmp[j].events = fd[i].events;
		tmp[j].revents = fd[i].revents;
		j++;
	}
	delete [] fd;
	size -= 1;
	return (tmp);
}

void ft_send(int fd, const char *str, size_t size, int flag)
{
	if (send(fd, str, size, flag) == -1)
	{
		throw std::runtime_error("Send failed");
	}
}

void	ft_close(struct pollfd *fd, size_t & size)
{
	for (size_t i = 0; i < size; i++)
	{
		close(fd[i].fd);
	}
	size = 0;
	delete [] fd;
}
