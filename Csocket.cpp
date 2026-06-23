#include "Csocket.hpp"

Csocket::Csocket(int domain, int type, int protocol)
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
	this->_fd = new struct pollfd;
	this->_fd[0].fd = fd;
	this->_fd[0].events = POLLIN;
	this->_fd[0].revents = 0;
	this->_size = 1;
}

Csocket::~Csocket(void)
{
	for (size_t i = 0; i < this->_size; i++)
	{
		close(this->_fd[i].fd);
	}
	delete [] this->_fd;
	this->_s.clear();
}

void Csocket::bind(in_addr_t ip_addr, in_port_t port, sa_family_t family)
{
	struct sockaddr_in s_bind;

	std::memset(&s_bind, 0, sizeof(s_bind));
	s_bind.sin_addr.s_addr = ip_addr;
	s_bind.sin_port = htons(port);
	s_bind.sin_family = family;
	if (::bind(this->_fd[0].fd, (struct sockaddr *)&s_bind, sizeof(s_bind)))
	{
		throw std::runtime_error("Bind failed");
	}
}

void Csocket::listen(int max_connection)
{
	if (::listen(this->_fd[0].fd, max_connection))
	{
		throw std::runtime_error("Listen failed");
	}
}

void Csocket::accept(void)
{

}


