#include "server.hpp"


void print_error_exit(std::string src,std::vector<struct pollfd>& listener)
{
    std::cerr << src << ": " << strerror(errno) << std::endl;
    for(size_t i = 0; i < listener.size(); i++)
        close(listener[i].fd);
    exit(1);
}

uint32_t ip_convert(std::string& ip)
{
    if(ip == "127.0.0.1")
        return 2130706433;
    else
        return INADDR_ANY;
}

void server::init(std::vector<t_config>& list)
{
    signal(SIGPIPE, SIG_IGN);
    for(size_t i = 0; i < list.size(); i++)
    {
        if(know_exist.count({list[i].ip, list[i].port}))
        {
            int fd = know_exist[{list[i].ip, list[i].port}];
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
            know_exist[{list[i].ip, list[i].port}] = fd;
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
    c.server = servers[listener_fd].data();
    clients[client] = c;
}

void server::cleaning_client(std::map<int, t_client>::iterator it)
{
    close(it->first);
    std::vector<struct pollfd>::iterator t = listeners.begin();
    while(t != listeners.end())
    {
        if(t->fd == it->first)
        {
            listeners.erase(t);
            break;
        }
        t++;
    }
    clients.erase(it);
}

void server::cleaning_cgi(int fd)
{
    close(fd);
    std::vector<struct pollfd>::iterator t = listeners.begin();
    while(t != listeners.end())
    {
        if(t->fd == fd)
        {
            listeners.erase(t);
            break;
        }
        t++;
    }
}

t_location* choose_location(t_config* server, std::string target)
{
    t_location* best_loc = NULL;
    size_t long_loc = 0;

    
    for(size_t i = 0; i < server->location.size(); i++)
    {
        std::string path = server->location[i].path;
        if(target.find(path) != 0)
            continue;
        bool exact_same = (target == path);
        bool target_end_slash = false;
        if(target.length() > path.length())
            target_end_slash = (target[path.length()] == '/'); 
        bool path_end_slash = false;
        if(path.length() > 0)
            path_end_slash = (path[path.length() - 1] == '/'); 

        if(!target_end_slash && !path_end_slash && !exact_same)
            continue;
        if( path.length() != 0 && long_loc < path.length())
        {
            best_loc = &server->location[i];
            long_loc = path.length();
        }
    } 
    return best_loc;
}

bool is_method_allow(t_location* loc, std::string method)
{
    if(method == "GET")
        return (loc->allow_method & GET);
    else if(method == "POST")
        return (loc->allow_method & POST);
    else if(method == "DELETE")
        return (loc->allow_method & DELETE);
    return false;
}

std::string get_extention(std::string target)
{
    std::string ext;
    size_t pos = target.rfind('.');
    if(pos == std::string::npos)
       return "";
    return target.substr(pos);
}

char **build_argv(t_client& client, std::string& cgi_handler)
{
    std::string target = client.location->root_path + client.request.target;
    char **argv = new char*[3];
    argv[0] = strdup(cgi_handler.c_str());
    argv[1] = strdup(target.c_str());
    argv[2] = NULL;
    return argv;
}

std::string to_string(size_t n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
}

char **build_env(t_client& client)
{
    char ** env = new char*[11];
    std::string tmp = "REQUEST_METHOD=" + client.request.method;
    env[0] = strdup(tmp.c_str());
    tmp = "QUERY_STRING=" + client.request.query_string;
    env[1] = strdup(tmp.c_str());
    tmp = "CONTENT_LENGTH=" + to_string(client.request.content_length);
    env[2] = strdup(tmp.c_str());

    tmp = "CONTENT_TYPE=";
    if(client.request.headers.count("Content-Type"))
        tmp += client.request.headers["Content-Type"];
    env[3] = strdup(tmp.c_str());

    tmp = "SCRIPT_FILENAME=" + (client.location->root_path + client.request.target);
    env[4] = strdup(tmp.c_str());
    tmp = "SCRIPT_NAME=" + client.request.target;
    env[5] = strdup(tmp.c_str());
    tmp = "SERVER_PROTOCOL=" + client.request.version;
    env[6] = strdup(tmp.c_str());
    tmp = "SERVER_NAME=" + client.request.server_name;
    env[7] = strdup(tmp.c_str());
    tmp = "SERVER_PORT=" + to_string(client.server->port);
    env[8] = strdup(tmp.c_str());
    env[9] = strdup("GATEWAY_INTERFACE=CGI/1.1");
    env[10] = NULL;
    return  env;
}

void free_arr(char **arr)
{
    for (int i = 0; arr[i]; i++)
        free(arr[i]);
    delete[] arr;
}


void server::start_cgi(t_client& client, std::string cgi_handler)
{
    int std_in[2];
    int std_out[2];
    
    if(pipe(std_out) < 0)
    {
        // queue 500 Internal Server Error
        return;
    }

    if(client.request.method == "POST")
    {
        if(pipe(std_in) < 0)
        {
            close(std_out[0]);
            close(std_out[1]);
            // queue 500 Internal Server Error
            return;
        }
    }


    pid_t pid = fork();
    if(pid < 0)
    {
        close(std_out[0]);
        close(std_out[1]);
        if(client.request.method == "POST")
        {
            close(std_in[0]);
            close(std_in[1]);
        }
        // queue 500 Internal Server Error
        return;
    }
    if(pid == 0)
    {
        if(client.request.method == "POST")
        {    
            if(dup2(std_in[0], 0) < 0)
                exit(1);
            close(std_in[0]);
            close(std_in[1]);
        }
        if(dup2(std_out[1], 1) < 0)
            exit(1);
        close(std_out[0]);
        close(std_out[1]);
        char **argv = build_argv(client, cgi_handler);
        char **env = build_env(client);
        execve(argv[0], argv, env);
        std::cerr << "execve: " << strerror(errno) << std::endl;
        free_arr(argv);
        free_arr(env);
        exit(1);
    }
    close(std_out[1]);
    t_CgiProcess c;
    c.pid = pid;
    c.out_fd = std_out[0];
    c.client_fd = client.fd;
    c.stdout_closed = false;
    if(client.request.method == "POST")
    {
        pollfd p;
        close(std_in[0]);
        p.events = POLLOUT;
        p.fd = std_in[1];
        p.revents = 0;
        fcntl(p.fd, F_SETFL, O_NONBLOCK);
        c.in_fd = std_in[1];
        c.stdin_closed = false;
        fd_to_pid[p.fd] = pid; 
        listeners.push_back(p);
    }
    else
    {
        c.stdin_closed = true;
        c.in_fd = -1;
    }
    pollfd p;
    p.events = POLLIN;
    p.fd = std_out[0];
    p.revents = 0;
    fcntl(p.fd, F_SETFL, O_NONBLOCK);
    fd_to_pid[p.fd] = pid;
    listeners.push_back(p);
    cgis[pid] = c;
    client.state = WAITING_CGI;
}

std::string getContentType(const std::string& target)
{
	size_t start_dot = target.find_last_of('.');
	
	if(start_dot != std::string::npos)
	{
		std::string type = target.substr(start_dot);
		if(type == ".html")
			return "text/html";
		else if(type == ".css")
			return "text/css";
		else if(type == ".js")
			return "text/javascript";
		else if(type == ".jpg")
			return "image/jpeg";
		else if(type == ".png")
			return "image/png";
		else if (type == ".py")
			return "text/x-python";
	}
	return "application/octet-stream";
}

bool server::get_index(t_client& client)
{
    struct stat st;

    for(int i = 0; i < client.location->index.size(); i++)
    {
        std::string file = client.location->root_path + client.request.target + "/" + client.location->index[i];
        if(stat(file.c_str(), &st) == 0)
        {
            serve_file(client, file);
            return true;
        }
    }
    return false;
}

void server::serve_file(t_client& client, const std::string& file_path)
{
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if(!file.is_open())
    {
        // 403 Forbidden
         return;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    client.response.body = content;
    client.response.status_code = 200;
    client.response.status = "OK";
    client.response.headers["Content-Type"] = getContentType(file_path);
    build_response(client);
    client.state = WRITING_RESPONSE;
    set_events(client.fd, POLLOUT);
}

void server::generate_index(t_client& client, const std::string& file_path)
{
    DIR *dir = opendir(file_path.c_str());
    if(!dir)
    {
        // 403
        return;
    }
    std::ostringstream html;

    html << "<html>\n";
    html << "<body>\n";
    html << "<h1>Index of " << client.request.target << "</h1>\n";
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL)
    {
        std::string name = entry->d_name;
        if(name == "." || name == "..")
            continue;
        html << "<a href=\"" << name << "\">" << name << "</a><br>\n";
    }
    html << "</body>\n";
    html << "</html>\n";
    closedir(dir);
    client.response.status_code = 200;
    client.response.status = "OK";
    client.response.body = html.str();
    client.response.headers["Content-Type"] = "text/html";
    build_response(client);
    client.state = WRITING_RESPONSE;
    set_events(client.fd, POLLOUT);
}

void server::handle_get(t_client& client)
{
    struct stat file_stat;
    std::string file_path;

    file_path =  client.location->root_path + client.request.target;

    if(stat(file_path.c_str(), &file_stat) < 0)
    {
        // 404 Not found
        return;
    }
    if(S_ISDIR(file_stat.st_mode))
    {
        if(get_index(client))
            return;
        else if(client.location->autoindex)
        {
            generate_index(client, file_path);
            return;
        }
        // 403 Forbidden
        return;
    }
    serve_file(client, file_path);
}

void server::handle_delete(t_client& client)
{
    struct stat st;
    std::string file = client.location->root_path + client.request.target;
    if(stat(file.c_str(), &st) == -1)
    {
        // 404 Not Found
        return;
    }
    if(S_ISDIR(st.st_mode))
    {
        // 403
        return;
    }
    if(remove(file.c_str()) != 0)
    {
        if(errno == EACCES)
        {
            // 403
        }
        else
        {
            //500
        }
        return;
    }
    client.response.status_code = 204;
    client.response.status = "No Content";
    client.response.body.clear();

    build_response(client);

    client.state = WRITING_RESPONSE;
    set_events(client.fd, POLLOUT);
}

void server::handle_post(t_client& client)
{
    
}


void server::route(t_client& client)
{
    t_config* srv = client.server;
    t_location* loc = choose_location(srv, client.request.target);
    if(loc == NULL)
    { 
        // queue_error(404) // NOT FOUND;
        return;
    }
    client.location = loc;
    if(!is_method_allow(loc,client.request.method))
    {
        // queue_error(405) Method not allow;
        return; 
    }
    if(!loc->return_path.empty())
    {
        int code = loc->return_path.begin()->first;
        std::string path = loc->return_path.begin()->second;
        //queue_redirect;
    }
    std::string ext = get_extention(client.request.target);
    if(loc->cgi_ext.count(ext))
    {
        start_cgi(client, loc->cgi_ext[ext]);
        return;
    }

    if(client.request.method == "GET")
        handle_get(client);
    else if(client.request.method == "POST")
        handle_post(cient);
}


void server::handle_client_read(int fd)
{
    char buffer[1024];
    int n = recv(fd, buffer, sizeof(buffer), 0);
    if(n < 0)
	{
		if(errno == EAGAIN || errno == EWOULDBLOCK)
			return;
		clients[fd].state = DEAD;
		return;
	}
    if(n == 0)
    {
        clients[fd].state = DEAD;
        return;
    }
    t_client& client = clients[fd];
    client.inbuf.append(buffer,n);
    if(client.state == READING_HEADERS)
    {
        size_t p = client.inbuf.find("\r\n\r\n");
        if(p != std::string::npos)
        {
            // call function parsing request;
			// if(!pars_request)
			// {
			//		prepare 400 respond
			//		state = writing_respond
			//		return;
			// }
			// call function chose_server(client);
            client.inbuf.erase(client.inbuf.begin(), client.inbuf.begin() + (p + 4));
            if(client.request.method == "POST")
                client.state = READING_BODY;
            else
                client.state = ROUTING;
        }
		else 
			return;
    }
    if(client.state == READING_BODY)
    {
        size_t n = client.inbuf.size();
        if(n > client.server->client_max_body_size)
            // make a respond error code 413 Payload Too Large
            ;
        if(n >= client.request.content_length)
        {
            client.request.body = client.inbuf.substr(0, client.request.content_length);
            client.inbuf.erase(0, client.request.content_length);
            client.state = ROUTING;
        }
    }
    if(client.state == ROUTING)
        route(client);
}

bool parse_cgi(std::string cgi_out, t_response& client)
{

}

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

void server::handle_cgi_read(int fd)
{
    t_CgiProcess& cgi = cgis[fd_to_pid[fd]];
    t_client& client = clients[cgi.client_fd];

    char buffer[4096];
    ssize_t n = read(fd, buffer, sizeof(buffer));
    if(n < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        cleaning_cgi(fd);
        fd_to_pid.erase(fd);
        cgi.stdout_closed = true;
        // 502 Bad Gateway
        return;
    }
    if(n == 0)
    {
        cleaning_cgi(fd);
        fd_to_pid.erase(fd);
        cgi.stdout_closed = true;
        if (!parse_cgi(cgi.outbuf, client.response))
        {
            // queue 502 Bad Gateway
            return;
        }
        client.state = WRITING_RESPONSE;
        set_events(fd, POLLOUT);
        return;
    }
    if(n > 0)
        cgi.outbuf.append(buffer, n);
}


void server::handle_cgi_write(int fd)
{
    t_CgiProcess& cgi = cgis[fd_to_pid[fd]];
    t_client& client = clients[cgi.client_fd];
    ssize_t n = write(cgi.in_fd, client.request.body.c_str(), client.request.body.size());
    if(n < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        cleaning_cgi(fd);
        fd_to_pid.erase(fd);
        cgi.stdin_closed = true;
        // queue 500
        return;
    }
    client.request.body.erase(0, n);
    
    if(client.request.body.size() == 0)
    {
        cleaning_cgi(fd);
        fd_to_pid.erase(fd);
        cgi.stdin_closed = true;
        return;
    }
}
void reset_client(t_client& client)
{
    client.request = s_request();
    client.response = s_respond();
    client.state = READING_HEADERS;
}

void server::build_response(t_client& client)
{

    std::ostringstream ss;
	
	ss << "HTTP/1.1 " << client.response.status_code  << " " << client.response.status +"\r\n";
    client.response.headers["Content-Length"] = to_string(client.response.body.size());
    
    if (!client.response.headers.count("Content-Type"))
        client.response.headers["Content-Type"] = "text/plain";

    if(client.keep_alive)
        client.response.headers["Connection"] = "keep-alive";
    else
        client.response.headers["Connection"] = "close";

    for(std::map<std::string, std::string>::iterator it = client.response.headers.begin(); it != client.response.headers.end(); ++it)
	{
        ss << it->first << ": " << it->second << "\r\n";
    }
    
    ss << "\r\n" << client.response.body;
    client.response.outbuf = ss.str();
}

void server::handle_client_write(int fd)
{
    t_client& client = clients[fd];
    ssize_t n = write(fd, client.response.outbuf.c_str(), client.response.outbuf.size());
    if(n < 0)
    {
        if(errno == EAGAIN || errno == EWOULDBLOCK)
            return;
        clients[fd].state = DEAD;
        return;
    }
    if(n > 0)
    {
        client.response.outbuf.erase(0, n);
 
        if(client.response.outbuf.empty())
        {
            if(client.keep_alive)
            {
                reset_client(client);
                set_events(fd, POLLIN);
            }
            else
                client.state = DEAD;
        }
        return;
    }
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
                    handle_client_read(listeners[i].fd);
                if(listeners[i].revents & POLLHUP)
                    clients[listeners[i].fd].state = DEAD;
                if (listeners[i].revents &  POLLOUT)
                    handle_client_write(listeners[i].fd);
				continue;
            }
            else if(cgis.count(fd_to_pid[listeners[i].fd]))
            {
                if(listeners[i].revents & POLLIN)
                    handle_cgi_read(fd_to_pid[listeners[i].fd]);
                else if(listeners[i].revents & POLLOUT)
                    handle_cgi_write(fd_to_pid[listeners[i].fd]);
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
                waitpid(t->second.pid, NULL, WNOHANG);
                cgis.erase(tmp);
            }
            else
                t++;
        }

    }
}
