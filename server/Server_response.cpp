#include "server.hpp"



std::string to_string(size_t n)
{
    std::stringstream ss;
    ss << n;
    return ss.str();
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

void reset_client(t_client& client)
{
    if (client.request.body_fd != -1) {
        close(client.request.body_fd);
    }
    if (!client.request.body_file.empty()) {
        unlink(client.request.body_file.c_str());
    }
    
    client.request = s_request();
    client.response = s_respond();
    client.state = READING_HEADERS;
}

void server::cleaning_client(std::map<int, t_client>::iterator it)
{
    char buf[4096];
    while (recv(it->first, buf, sizeof(buf), MSG_DONTWAIT) > 0) {}
    
    if (it->second.request.body_fd != -1) {
        close(it->second.request.body_fd);
    }
    if (!it->second.request.body_file.empty()) {
        unlink(it->second.request.body_file.c_str());
    }
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

void server::build_response(t_client& client)
{

    std::ostringstream ss;
	ss << "HTTP/1.1 " << client.response.status_code  << " " << client.response.status +"\r\n";
    client.response.headers["Content-Length"] = to_string(client.response.body.size());
    
    if (!client.response.headers.count("Content-Type"))
        client.response.headers["Content-Type"] = "text/plain";


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
    ssize_t n = write(fd, client.response.outbuf.c_str() + client.response.bytes_sent, client.response.outbuf.size() - client.response.bytes_sent);
    if(n < 0 || n == 0)
    {
        clients[fd].state = DEAD;
        return;
    }
    if(n > 0)
    {
        client.response.bytes_sent += n;

        if(client.response.bytes_sent >= client.response.outbuf.size())
        {
            client.state = DEAD;
        }
        return;
    }
}

