#include "server.hpp"


void print_error_exit(std::string src,std::vector<struct pollfd>& listener)
{
    std::cerr << src << ": " << strerror(errno) << std::endl;
    for(size_t i = 0; i < listener.size(); i++)
        close(listener[i].fd);
    exit(1);
}


void server::queue_error(t_client& client, int code)
{
    std::string path;
    if(client.server->error_pages.count(code))
    {
        if(client.location)
            path = client.location->root_path + client.server->error_pages[code];
        else
            path = client.server->root_path + client.server->error_pages[code];
        
        serve_file(client, path, code);
        return;
    }
    std::ostringstream html;

    html << "<html>";
    html << "<head><title>";
    html << code << " ";
    html << reason_sentence(code);
    html << "</title></head>";

    html << "<body>";

    html << "<h1>";
    html << code << " ";
    html << reason_sentence(code);
    html << "</h1>";

    html << "</body></html>";

    client.response.status_code = code;
    client.response.status = reason_sentence(code);
    client.response.body = html.str();
    client.response.headers["Content-Type"] = "text/html";

    build_response(client);
    client.state = WRITING_RESPONSE;
    set_events(client.fd, POLLOUT);
}

void server::queue_redirect(t_client& client, const std::string& location, int code)
{
    client.response.status_code = code;
    client.response.status = reason_sentence(code);
    client.response.body.clear();
    client.response.headers["Location"] = location;

    build_response(client);

    client.state = WRITING_RESPONSE;

    set_events(client.fd, POLLOUT);
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
    tmp = "SERVER_PORT=" + to_string(client.request.server->port);
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
        queue_error(client, 500);
        return;
    }

    if(client.request.method == "POST")
    {
        if(pipe(std_in) < 0)
        {
            close(std_out[0]);
            close(std_out[1]);
            queue_error(client, 500);
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
        queue_error(client, 500);
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
    set_events(client.fd, 0);
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

    for(size_t i = 0; i < client.location->index.size(); i++)
    {
        std::string file = client.location->root_path + client.request.target + "/" + client.location->index[i];
        if(stat(file.c_str(), &st) == 0)
        {
            serve_file(client, file, 200);
            return true;
        }
    }
    return false;
}

std::string server::reason_sentence(int code)
{
    switch(code)
    {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 413: return "Payload Too Large";
        case 500: return "Internal Server Error";
        case 501: return "Not Implemented";
        case 502: return "Bad Gateway";
        default:  return "Unknown";
    }
}

void server::serve_file(t_client& client, const std::string& file_path, int code)
{
    std::ifstream file(file_path.c_str(), std::ios::in | std::ios::binary);
    if(!file.is_open())
    {
        queue_error(client, 403);
        return;
    }
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    client.response.body = content;
    client.response.status_code = code;
    client.response.status = reason_sentence(code);
    client.response.headers["Content-Type"] = getContentType(file_path);
    build_response(client);
    client.state = WRITING_RESPONSE;
    set_events(client.fd, POLLOUT);
    file.close();
}

void server::generate_index(t_client& client, const std::string& file_path)
{
    DIR *dir = opendir(file_path.c_str());
    if(!dir)
    {
        queue_error(client, 403);
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
        queue_error(client, 404);
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
        queue_error(client, 403);
        return;
    }
    serve_file(client, file_path, 200);
}

void server::handle_delete(t_client& client)
{
    struct stat st;
    std::string relative = client.request.target.substr(client.location->path.size());
    std::string root_location =  client.location->root_path.substr(1);
    std::string file = client.server->root_path + root_location + "/" + relative;
    if(stat(file.c_str(), &st) == -1)
    {
        queue_error(client, 404);
        return;
    }
    if(S_ISDIR(st.st_mode))
    {
        queue_error(client, 403);
        return;
    }
    if(remove(file.c_str()) != 0)
    {
        if(errno == EACCES)
        {
            queue_error(client, 403);
        }
        else
        {
            queue_error(client, 500);
        }
        return;
    }
    client.response.status_code = 204;
    client.response.status = "No Content";
    client.response.body = "File deleted!";
    client.response.headers["Content-Type"] = "text/plain";
    build_response(client);
    client.state = WRITING_RESPONSE;
    set_events(client.fd, POLLOUT);
}

void server::handle_post(t_client& client)
{
    std::string relative = client.request.target.substr(client.location->path.size());
    std::string root_location =  client.location->root_path.substr(1);
    std::string file_path = client.server->root_path + root_location + "/" + relative;
    std::ofstream file(file_path.c_str(), std::ios::out | std::ios::binary);
    if(!file.is_open())
    {
        queue_error(client, 500);
        return;
    }
    file << client.request.body;
    file.close();
       
    client.response.status_code = 201;
    client.response.status = "Created";
    client.response.body = "File uploaded successfully!";
    client.response.headers["Content-Type"] = "text/plain";
    build_response(client);
    client.state = WRITING_RESPONSE;
    set_events(client.fd, POLLOUT);
}

void server::route(t_client& client)
{
    t_config* srv = client.request.server;
    t_location* loc = choose_location(srv, client.request.target);
    if(loc == NULL)
    { 
        queue_error(client, 404);
        return;
    }
    client.location = loc;
    if(!is_method_allow(loc,client.request.method))
    {
        queue_error(client, 405);
        return; 
    }
    if(!loc->return_path.empty())
    {
        int code = loc->return_path.begin()->first;
        std::string path = loc->return_path.begin()->second;
        queue_redirect(client, path, code);
        return;
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
        handle_post(client);
    else if (client.request.method == "DELETE")
        handle_delete(client);
    return;
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
            
			 if(!parse_request(client.request, client.inbuf))
			{
				queue_error(client, 400);
				return;
			}
			choose_server(client);
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
            queue_error(client, 413);
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

bool parse_cgi(const std::string &out, t_response &res)
{
	std::string tmp;
	std::string delim = "\r\n";
	size_t		pos;
	int			pos_len = 4;

	res.status_code = 200;
	res.status = "OK";
	pos = out.find("\r\n\r\n");
	if (pos == std::string::npos)
	{
		pos = out.find("\n\n");
		if (pos == std::string::npos)
			return (false);
		pos_len = 2;
		delim = "\n";
	}
	tmp = out.substr(0, pos);
	res.body = out.substr(pos + pos_len);
	if (tmp != "")
	{
		if (!ft_cgi(tmp, delim, res))
			return (false);
	}
	return (true);
}

bool server::parse_request(t_request &req, const std::string &buffer)
{
	std::string tmp;
	size_t	pos;

	pos = buffer.find("\r\n");
	if (pos == std::string::npos)
		return (false);
	tmp = buffer.substr(0, pos);
	if (!ft_method(req, tmp))
		return (false);
	tmp = buffer.substr(pos + 2);
	if (!ft_headers(req, tmp))
		return (false);
	req.body = tmp;
	req.body_received = tmp.length();
	return (true);
}

void server::choose_server(t_client &client)
{
    std::string host = client.request.headers["Host"];

    // remove optional :port
    size_t pos = host.find(':');
    if (pos != std::string::npos)
        host.erase(pos);

    std::vector<t_config> &v = servers[client.server_fd];

    for (size_t i = 0; i < v.size(); i++)
    {
        for (size_t j = 0; j < v[i].server_name.size(); j++)
        {
            if (v[i].server_name[j] == host)
            {
                client.request.server = &v[i];
                client.request.server_name = host;
                return;
            }
        }
    }
    client.request.server = client.server;
    // no matching Host → keep the default server
    client.request.server_name = client.server->server_name[0];
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
        queue_error(client, 502);
        return;
    }
    if(n == 0)
    {
        cleaning_cgi(fd);
        fd_to_pid.erase(fd);
        cgi.stdout_closed = true;
        if (!parse_cgi(cgi.outbuf, client.response))
        {
            queue_error(client, 502);
            return;
        }
        build_response(client);
        client.state = WRITING_RESPONSE;
        set_events(client.fd, POLLOUT);
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
        queue_error(client, 500);
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
