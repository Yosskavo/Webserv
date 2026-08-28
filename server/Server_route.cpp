#include "server.hpp"


std::string get_extention(std::string target)
{
    std::string ext;
    size_t pos = target.rfind('.');
    if(pos == std::string::npos)
       return "";
    return target.substr(pos);
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

void server::route(t_client& client)
{
    struct stat st;
    std::string file_path = client.server->root_path + client.request.target;
	std::cout << "file Path + client.servver : " + file_path << std::endl;

    if (stat(file_path.c_str(), &st) == 0 && S_ISDIR(st.st_mode))
    {
        if (client.request.target[client.request.target.size() - 1] != '/')
        {
            queue_redirect(client, client.request.target + "/", 301);
            return;
        }
    }  
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
        std::string target_file = loc->root_path;
        if (target_file[target_file.size() - 1] != '/')
            target_file += "/";
        target_file += client.request.target.substr(loc->path.size());
        
        struct stat st_cgi;
        if (stat(target_file.c_str(), &st_cgi) != 0 || S_ISDIR(st_cgi.st_mode))
        {
            queue_error(client, 404);
            return;
        }

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
