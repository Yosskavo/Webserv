#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

int main()
{
	int fd_socket;
	int fd_client;
	int opt = 1;
	struct sockaddr_in	s_socket;
	struct sockaddr_in	s_accept;
	socklen_t l_socket = sizeof(s_accept);
	char	str[1024];
	std::string			s;
	int			tmp;

	fd_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_socket < 0)
	{
		std::cerr << "socket faild : " << std::endl;
		return (1);
	}
	if (setsockopt(fd_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		std::cerr << "setsockopt failed" << std::endl;
		close(fd_socket);
		return (1);
	}
	std::cout << "Create a socket : " << fd_socket << std::endl;
	std::memset(&s_socket, 0, sizeof(s_socket));
	s_socket.sin_addr.s_addr = INADDR_ANY;
	s_socket.sin_port = htons(8080);
	s_socket.sin_family = AF_INET;
	if (bind(fd_socket, (struct sockaddr *)&s_socket, sizeof(s_socket)) == -1)
	{
		std::cerr << "bind failed : " << std::endl;
		close(fd_socket);
		return (1);
	}
	std::cout << "Bind to the socket" << std::endl;
	if (listen(fd_socket, SOMAXCONN))
	{
		std::cerr << "listen faild : " << std::endl;
		close(fd_socket);
		return (1);
	}
	std::memset(&s_accept, 0, sizeof(s_accept));
	fd_client = accept(fd_socket, (struct sockaddr *)&s_accept, &l_socket);
	if (fd_client == -1)
	{
		std::cerr << "accept faild : " << std::endl;
		close(fd_socket);
		return (1);
	}
	std::cout << "Accept client : " << fd_client << std::endl;
	std::cout << "Waiting for client to send" << std::endl;
	while (true)
	{
		std::memset(&str, 0, sizeof(str));
		tmp = recv(fd_client, &str, 1024, 0);
		if (!tmp)
		{
			std::cerr << "Client Disconnected" << std::endl;
			close(fd_client);
			break ;
		}
		else if (tmp == -1)
		{
			std::cerr << "resv faild : " << std::endl;
			close(fd_client);
			break;
		}
		s = str;
		std::cout << "Client : " +s;
		s = "Delivered\n";
		// TODO: implement poll();

		// tmp = send(fd_client, s.c_str(), s.size(), 0);
		// if (tmp == -1)
		// {
		// 	std::cerr << "send faild : " << std::endl;
		// 	close(fd_client);
		// 	break ;
		// }
		s.clear();
	}
	close(fd_socket);
	std::cout << "Exit" << std::endl;
}
