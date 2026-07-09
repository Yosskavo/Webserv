#include "server.hpp"


void print_error_exit(std::string src,std::vector<struct pollfd>& listener)
{
    std::cerr << src << ": " << strerror(errno) << std::endl;
    for(int i = 0; i < listener.size(); i++)
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
    for(int i = 0; i < list.size(); i++)
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

void server::cleaning_client(int fd)
{
    close(fd);
    std::vector<struct pollfd>::iterator it = listeners.begin();
    while(it != listeners.end())
    {
        if(it->fd == fd)
        {
            listeners.erase(it);
            break;
        }
        it++;
    }
    clients.erase(fd);
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

void start_cgi(t_client& client, std::string cgi_handler)
{
    
}


void route(t_client& client)
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
}


void server::handle_client_read(int fd)
{
    char buffer[1024];
    int n = recv(fd, buffer, sizeof(buffer), 0);
    if(n < 0)
        return;
    if(n == 0)
    {
        cleaning_client(fd);
        return;
    }
    t_client& client = clients[fd];
    client.inbuf.append(buffer);
    if(client.state == READING_HEADERS)
    {
        size_t p = client.inbuf.find("\r\n\r\n");
        if(p != std::string::npos)
        {
            // call function parsing request;
            // call function chose_server(client);
            client.inbuf.erase(client.inbuf.begin(), client.inbuf.begin() + (p + 4));
            if(client.request.method == "POST")
                client.state = READING_BODY;
            else
                client.state = ROUTING;
        }
    }
    else if(client.state == READING_BODY)
    {
        size_t n = client.inbuf.size();
        if(n > client.server->client_max_body_size)
            // make a respond error code 403
            ;
        if(n >= client.request.content_length)
        {
            client.request.body = client.inbuf.substr(0, client.request.content_length);
            client.inbuf.clear();
            client.state = ROUTING;
        }
    }
    else if(client.state == ROUTING)
        route(client);
}

void server::run()
{
    while(1)
    {
        poll(listeners.data(), listeners.size(), -1);
        for(int i = 0; i < listeners.size(); i++)
        {
            if(servers.count(listeners[i].fd))
            {
                if(listeners[i].revents | POLLIN)
                   accept_new_clients(listeners[i].fd); 
            }
            else if(clients.count(listeners[i].fd))
            {
                if(listeners[i].revents | POLLIN)
                    handle_client_read(listeners[i].fd);
                else if (listeners[i].revents | POLLOUT)
                    handle_client_write(listeners[i].fd);
                else
                    continue;
            }
                
        }       
    }
}