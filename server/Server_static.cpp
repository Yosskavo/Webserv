#include "server.hpp"

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
		else if (type == ".php")
			return "application/x-httpd-php";
	}
	return "application/octet-stream";
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

bool server::get_index(t_client& client)
{
    struct stat st;
    std::string relative = client.request.target.substr(client.location->path.size());
    std::string current_dir = client.location->root_path + "/" + relative;
    size_t pos;
    while((pos = current_dir.find("//")) != std::string::npos)
        current_dir.replace(pos, 2, "/");
    for(size_t i = 0; i < client.location->index.size(); i++)
    {
        std::string file = current_dir + "/" + client.location->index[i];
        if(stat(file.c_str(), &st) == 0)
        {
            serve_file(client, file, 200);
            return true;
        }
    }
    return false;
}

void server::serve_file(t_client& client, const std::string& file_path, int code)
{
    std::ifstream file(file_path.c_str(), std::ios::in | std::ios::binary);
    if(!file.is_open())
    {
        queue_error(client, 403); // manual error; 
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

void server::handle_get(t_client& client)
{
    struct stat file_stat;
    std::string file_path;
    std::string relative = client.request.target.substr(client.location->path.size());
    file_path =  client.location->root_path + "/" + relative;
    size_t pos;
    while((pos = file_path.find("//")) != std::string::npos)
        file_path.replace(pos, 2, "/");
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
        queue_error(client, 404);
        return;
    }
    serve_file(client, file_path, 200);
}

