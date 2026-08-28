#include "server.hpp"


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
	if (client.request.server_name.empty())
		client.request.server_name = "";
	else
		client.request.server_name = client.server->server_name[0];
}


bool server::parse_request(t_request &req, const std::string &buffer)
{
	std::string tmp;
	size_t	pos;

	std::cout << "This is the request : " + buffer << std::endl;
	pos = buffer.find("\r\n");
	if (pos == std::string::npos)
		return (false);
	tmp = buffer.substr(0, pos);
	if (!ft_method(req, tmp))
		return (false);
	tmp = buffer.substr(pos + 2);
	if (!ft_headers(req, tmp))
		return (false);
	if(req.headers.count("Transfer-Encoding") && req.headers["Transfer-Encoding"] == "chunked")
        req.is_chunked = true;
    req.body = tmp;
	req.body_received = tmp.length();
	return (true);
}



void server::handle_client_read(int fd)
{
    char buffer[1024];
    int n = recv(fd, buffer, sizeof(buffer), 0);
    if(n < 0 || n == 0)
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
        if(client.request.body_fd == -1)
        {
            std::stringstream ss;
            ss << "/tmp/webserv_body_" << client.fd << "_" << rand();
            client.request.body_file = ss.str();
            client.request.body_fd = open(client.request.body_file.c_str(), O_CREAT| O_WRONLY | O_TRUNC, 0644);
        }
        if(client.request.is_chunked)
        {
            
            while(1)
            {
                size_t p = client.inbuf.find("\r\n");
                if(p == std::string::npos)
                    break;
                std::string hex = client.inbuf.substr(0, p);
                size_t chunk_size = strtol(hex.c_str(), NULL, 16);
                
                if(chunk_size == 0)
                {
                    client.inbuf.erase(0, p + 4);
                    client.state = ROUTING;
                    break;
                }
                
                if(client.inbuf.size() >= p + 2 + chunk_size + 2)
                {
                    std::string chunk = client.inbuf.substr(p+2, chunk_size);
                    write(client.request.body_fd, chunk.c_str(), chunk.size());
                    client.request.content_length += chunk_size;

                    t_location* loc = choose_location(client.request.server, client.request.target);
                    size_t max_body = loc ? loc->client_max_body_size : client.request.server->client_max_body_size;
                    
                    if(client.request.content_length > max_body)
                    {
                        queue_error(client, 413); 
                        return;
                    }
                    client.inbuf.erase(0, p + 2 + chunk_size + 2);
                }
                else
                    break;
            }
        }
        else
        {
            t_location* loc = choose_location(client.request.server, client.request.target);
            size_t max_body = loc ? loc->client_max_body_size : client.request.server->client_max_body_size;
            
            size_t n = client.inbuf.size();
            if(n > max_body)
            {
                queue_error(client, 413);
                return;
            }
            if(n >= client.request.content_length)
            {
                client.request.body = client.inbuf.substr(0, client.request.content_length);
                write(client.request.body_fd, client.request.body.c_str(), client.request.body.size());
                client.inbuf.erase(0, client.request.content_length);
                client.state = ROUTING;
            }
        }
    }
    if(client.state == ROUTING)
        route(client);
}