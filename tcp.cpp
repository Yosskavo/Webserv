#include "tcp.h"
#include <stdio.h>

int	ft_init()
{
	int fd_socket;

	fd_socket = ft_socket(AF_INET, SOCK_STREAM, 0);
	std::cout << "Create a socket : " << fd_socket << std::endl;
	ft_bind(fd_socket, INADDR_ANY, 8080, AF_INET);
	std::cout << "Bind to the socket" << std::endl;
	ft_listen(fd_socket, SOMAXCONN);
	std::cout << "Listen is set" << std::endl;
	return (fd_socket);
}

int main()
{
	// NOTE: should i create first class
	int fd_socket = -1;
	int	fd_ready = 0;
	struct pollfd		*fd;
	size_t				size = 1;
	int					f;
	std::string				str;
	std::map<int, std::string>	m;

	fd = new struct pollfd[1];
	try 
	{
		fd_socket = ft_init();
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << std::endl;
		if (fd_socket > -1)
			close(fd_socket);
		return (1);
	}
	fd[0].fd = fd_socket;
	fd[0].revents = 0;
	fd[0].events = POLLIN;
	while (true)
	{
		fd_ready = poll(fd, size, -1);
		if (fd_ready == -1)
		{
			std::cerr << "Poll faild" << std::endl;
			perror("Poll");
			return (1);
		}
		if (fd_ready == 0)
		{
			std::cerr << "Poll faild" << std::endl;
			return (1);
		}
		for (size_t i = 0; i < size; i++)
		{
			if (fd[i].revents & POLLHUP)
			{
				std::cout << "The client " << fd[i].fd << " Hung up" << std::endl;
				fd = ft_erase(i, fd, size);
			}
			else if (fd[i].revents & POLLERR && !i)
			{
				std::cout << "An Error accord with the client " << fd[i].fd << std::endl;
			}
			else if (fd[i].revents & POLLERR)
			{
				std::cout << "An Error accord with the client " << fd[i].fd << std::endl;
			}
			else if (fd[i].fd == fd_socket && fd[i].revents & POLLIN)
			{
				std::cout << "A new request received" << std::endl;
				fd = ft_accept(fd, size);
			}
			else if (fd[i].revents & POLLIN)
			{
				std::cout << "Client " << fd[i].fd << " is ready to send a data" << std::endl;
				if (m.find(fd[i].fd) == m.end())
					m[fd[i].fd] = "";
				try {
					f = ft_resv(fd[i].fd, m[fd[i].fd], 0, fd[i].revents);
				}
				catch (const std::exception & e)
				{
					std::cout << e.what() << std::endl;
				}
				if (!f)
				{
					std::cout << "Client " << fd[i].fd << " is disconnected" << std::endl;
					m.erase(i);
					fd = ft_erase(fd[i].fd, fd, size);
				}
			}
		}
	}
	ft_close(fd, size);
	std::cout << "exit" << std::endl;
}
