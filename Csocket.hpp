#ifndef CSOCKET_HPP
#define CSOCKET_HPP

#include "tcp.h"

class Csocket
{
	private :
		struct pollfd *				_fd;
		size_t						_size;
		std::map<int, std::string> 	_s;
	public  :
		Csocket(int domain, int type, int protocol);
		~Csocket(void);
		void bind(in_addr_t ip, in_port_t port, sa_family_t family);
		void listen(int max_connection);
		void accept(void);
};

#endif
