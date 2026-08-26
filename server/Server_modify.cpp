#include "server.hpp"

void server::handle_delete(t_client& client)
{
    struct stat st;
    std::string relative = client.request.target.substr(client.location->path.size());
    std::string root_location =  client.location->root_path.substr(1);
    std::string file = root_location + "/" + relative;
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
    //if location->root.empty
    std::string file_path = client.location->root_path  + "/" + relative;
    //else
    //std::string file_path = client.location->root_path; //always check the /
    struct stat st;
    if (stat(file_path.c_str(), &st) == 0)
    {
        if (S_ISDIR(st.st_mode))
        {
            client.response.status_code = 200;
            client.response.status = "OK";
            client.response.body = "OK";
            client.response.headers["Content-Type"] = "text/plain";
            build_response(client);
            client.state = WRITING_RESPONSE;
            set_events(client.fd, POLLOUT);
            return;
        }
    }
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