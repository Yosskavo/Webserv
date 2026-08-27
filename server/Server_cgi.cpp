#include "server.hpp"


void free_arr(char **arr)
{
    for (int i = 0; arr[i]; i++)
        free(arr[i]);
    delete[] arr;
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

char **build_env(t_client& client)
{
    std::vector<std::string> envs;
	std::string target = client.location->root_path;

    envs.push_back("REQUEST_METHOD=" + client.request.method);
    envs.push_back("QUERY_STRING=" + client.request.query_string);
    envs.push_back("CONTENT_LENGTH=" + to_string(client.request.content_length));
    
    if(client.request.headers.count("Content-Type"))
	{
		envs.push_back("CONTENT_TYPE=" + client.request.headers["Content-Type"]);
	}

	if (target[target.size() - 1] != '/')
		target += "/";
    target += client.request.target.substr(client.location->path.size());
    // std::cout << "target cgi env " + target << std::endl;
    envs.push_back("SCRIPT_FILENAME=" + target);
    envs.push_back("SCRIPT_NAME=" + client.request.target);
    envs.push_back("PATH_INFO=" + client.request.target);
    envs.push_back("REQUEST_URI=" + client.request.target);
    envs.push_back("SERVER_PROTOCOL=" + client.request.version);
    envs.push_back("SERVER_NAME=" + client.request.server_name);
    envs.push_back("SERVER_PORT=" + to_string(client.request.server->port));
    envs.push_back("GATEWAY_INTERFACE=CGI/1.1");
    envs.push_back("REDIRECT_STATUS=200");
    
    if (client.request.cookies.size())
        envs.push_back("HTTP_COOKIE=" + ft_join_the_map(client.request.cookies));

    // add all HTTP_ headers
    for(std::map<std::string, std::string>::iterator it = client.request.headers.begin(); it != client.request.headers.end(); ++it)
    {
        std::string key = it->first;
        for (size_t i = 0; i < key.size(); ++i) {
            if (key[i] == '-') key[i] = '_';
            else key[i] = toupper(key[i]);
        }
        envs.push_back("HTTP_" + key + "=" + it->second);
    }
    
    char ** env = new char*[envs.size() + 1];
    for (size_t i = 0; i < envs.size(); ++i)
        env[i] = strdup(envs[i].c_str());
    env[envs.size()] = NULL;
    return env;
}

char **build_argv(t_client& client, std::string& cgi_handler)
{
	std::string target = client.location->root_path;

	if (target[target.size() - 1] != '/')
		target += "/";
    target += client.request.target.substr(client.location->path.size());

    char **argv = new char*[3];
    argv[0] = strdup(cgi_handler.c_str());
    argv[1] = strdup(target.c_str());
    argv[2] = NULL;
    return argv;
}

void server::start_cgi(t_client& client, std::string cgi_handler)
{
    int std_out[2];
 
    if(pipe(std_out) < 0)
    {
        queue_error(client, 500);
        return;
    }


    pid_t pid = fork();
    if(pid < 0)
    {
        close(std_out[0]);
        close(std_out[1]);
        queue_error(client, 500);
        return;
    }
    if(pid == 0)
    {
        if(client.request.method == "POST")
        {    
            // Open the temp file we saved earlier and plug it into stdin (fd 0)
            int file_fd = open(client.request.body_file.c_str(), O_RDONLY);
            if (file_fd >= 0) 
            {
                dup2(file_fd, 0);
                close(file_fd);
            }
            else
                exit(1);
        }
        else
        {
            int dev_null = open("/dev/null", O_RDONLY);
            if(dev_null >= 0)
            {
                dup2(dev_null, 0);
                close(dev_null);
            }
            else
                close(0);
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
    c.stdin_closed = true;
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