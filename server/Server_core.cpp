#include "server.hpp"

void server::set_events(int fd, short events)
{
    for (size_t i = 0; i < listeners.size(); i++)
    {
        if (listeners[i].fd == fd)
        {
            listeners[i].events = events;
            return;
        }
    }
}

uint32_t ip_convert(std::string& ip)
{
    if(ip == "127.0.0.1")
        return 2130706433;
    else
        return INADDR_ANY;
}

void print_error_exit(std::string src,std::vector<struct pollfd>& listener)
{
    std::cerr << src << ": " << strerror(errno) << std::endl;
    for(size_t i = 0; i < listener.size(); i++)
        close(listener[i].fd);
    exit(1);
}

void server::init(std::vector<t_config>& list)
{
    signal(SIGPIPE, SIG_IGN);
    for(size_t i = 0; i < list.size(); i++)
    {
        std::pair<std::string, int> key(list[i].ip, list[i].port);
        if(know_exist.count(key))
        {
            int fd = know_exist[key];
            servers[fd].push_back(list[i]);
        }
        else
        {
            int fd = socket(AF_INET, SOCK_STREAM, 0);
            if(fd == -1)
                print_error_exit("socket",listeners);
            fcntl(fd, F_SETFL, O_NONBLOCK);
            int opt = 1;
            if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR,&opt, sizeof(opt)) == -1)
                print_error_exit("setsockopt",listeners);
            struct sockaddr_in info;
            memset(&info, 0, sizeof(info));
            info.sin_family = AF_INET;
            info.sin_port = htons(list[i].port);
            info.sin_addr.s_addr = htonl(ip_convert(list[i].ip));
            if(bind(fd, (const sockaddr *)&info, sizeof(info)) == -1)
                print_error_exit("bind",listeners);
            if(listen(fd, SOMAXCONN) == -1)
                print_error_exit("listen",listeners);
            struct pollfd p;
            p.fd = fd;
            p.events = POLLIN;
            p.revents = 0;
            listeners.push_back(p);
            know_exist[key] = fd;
            servers[fd].push_back(list[i]);
        }
    }  
}



void server::accept_new_clients(int listener_fd)
{

    int client = accept(listener_fd, NULL, 0);
    if(client == -1)
    {
        std::cerr << "accept: " << strerror(errno) << std::endl;
        return;
    }
    fcntl(client, F_SETFL, O_NONBLOCK);
    //add to list of listeners for poll
    struct pollfd p;
    p.fd = client;
    p.events = POLLIN;
    p.revents = 0;
    listeners.push_back(p);
    // add to list of clients
    t_client c;
    c.fd = client;
    c.state = READING_HEADERS;
    c.server_fd = listener_fd;
    c.server = &servers[listener_fd][0];
    clients[client] = c;
}



void server::run()
{
    while(1)
    {
        int n = poll(listeners.data(), listeners.size(), -1);
		if(n < 0)
		{
			if(errno == EINTR)
				continue;
			else
			{
				std::cerr << "poll: " << strerror(errno) << std::endl;
				break;
			}
		}
        for(size_t i = 0; i < listeners.size(); i++)
        {
            if(servers.count(listeners[i].fd))
            {
                if(listeners[i].revents & POLLIN)
                   accept_new_clients(listeners[i].fd); 
            }
            else if(clients.count(listeners[i].fd))
            {
				if(listeners[i].revents & (POLLERR|POLLNVAL))
				{
					clients[listeners[i].fd].state = DEAD;
					continue;
				}
				if(listeners[i].revents & POLLIN)
				{
					if(clients[listeners[i].fd].state != WAITING_CGI)
						handle_client_read(listeners[i].fd);
				}
                if(listeners[i].revents & POLLHUP)
                    clients[listeners[i].fd].state = DEAD;
                if (listeners[i].revents &  POLLOUT)
                    handle_client_write(listeners[i].fd);
				continue;
            }
            else if(fd_to_pid.count(listeners[i].fd) && cgis.count(fd_to_pid[listeners[i].fd]))
            {
                if(listeners[i].revents & (POLLERR | POLLNVAL))
                {
                    int fd = listeners[i].fd;
                    t_CgiProcess& cgi = cgis[fd_to_pid[fd]];
                    t_client& client = clients[cgi.client_fd];
                    if (fd == cgi.in_fd) cgi.stdin_closed = true;
                    if (fd == cgi.out_fd) cgi.stdout_closed = true;
                    cleaning_cgi(fd);
                    fd_to_pid.erase(fd);
                    queue_error(client, 500);
                }
                else if(listeners[i].revents & (POLLIN | POLLHUP))
                    handle_cgi_read(listeners[i].fd);
                else if(listeners[i].revents & POLLOUT)
                    handle_cgi_write(listeners[i].fd);
                continue;
            } 
		}
		std::map<int, t_client>::iterator it  = clients.begin();
		while(it != clients.end())
		{
			if(it->second.state == DEAD)
			{
				std::map<int, t_client>::iterator tmp  =  it;
				it++;
				cleaning_client(tmp);
			}
			else
				it++;
		}
        std::map<pid_t, t_CgiProcess>::iterator t = cgis.begin();
        while(t != cgis.end())
        {
            if(t->second.stdin_closed && t->second.stdout_closed)
            {
                std::map<pid_t, t_CgiProcess>::iterator tmp = t;
                t++;
                waitpid(tmp->second.pid, NULL, WNOHANG);
                cgis.erase(tmp);
            }
            else
                t++;
        }

    }
}
