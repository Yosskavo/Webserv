#ifndef SERVER_HPP
#define SERVER_HPP

#include "webserv.h"
#include <signal.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <poll.h>
#include <cstring>
#include <cerrno>
#include <sys/wait.h>
#include <sys/stat.h>
#include <dirent.h>


enum ClientState
{

    READING_HEADERS,
    READING_BODY,
    WRITING_RESPONSE,
	DEAD,
    WAITING_CGI,
    ROUTING,
    DONE

};

typedef struct s_request
{
    std::string method;
    std::string target;
    std::string version;
    std::string query_string;
    std::string server_name;
    t_config *server;
    std::map<std::string, std::string> cookies;
    std::map<std::string, std::string> headers;
    std::string body;
    size_t content_length;
    size_t body_received;
    s_request(): server(NULL), content_length(0),body_received(0){}

}t_request;

typedef struct s_respond
{
    int status_code;
    std::string status;
    std::map<std::string, std::string> headers;
    std::string body;
    std::string outbuf;
    size_t sent;
    s_respond() : status_code(0), sent(0){}

}t_response;

typedef struct s_client
{
    int fd;
    enum ClientState state;
    std::string inbuf;
    std::string outbuf;
    t_request request;
    t_response response;
    bool keep_alive;
    int server_fd;
    t_config* server;
    t_location *location;

    s_client() : fd(-1), state(READING_HEADERS), keep_alive(false), server_fd(-1), server(NULL), location(NULL) {}

}t_client;


typedef struct s_CgiProcess
{
    pid_t pid;
    int in_fd;              
    int out_fd;              
    size_t to_child;    // bytes still to write
    size_t from_child;         // bytes read so far
    int client_fd;
    bool stdin_closed;
    bool stdout_closed;
    std::string outbuf;
    //started_at
}t_CgiProcess;



class server
{
private:
    std::map <int, std::vector<t_config> > servers;
    std::vector<struct pollfd> listeners;
    std::map<std::pair<std::string, size_t> , int> know_exist;
    std::map<int, t_client> clients;
    std::map<pid_t, t_CgiProcess> cgis;
    std::map<int , pid_t> fd_to_pid;

public:
    server(){};
    ~server(){};
    void init(std::vector<t_config>& list);
    void run();
    void accept_new_clients(int listener_fd);
    void handle_client_read(int fd);
    void handle_client_write(int fd);
    void cleaning_client(std::map<int, t_client>::iterator it);
    void start_cgi(t_client& client, std::string cgi_handler);
    void handle_cgi_write(int fd);
    void handle_cgi_read(int fd);
    void cleaning_cgi(int fd);
    void route(t_client& client);
    void set_events(int fd, short events);
    void build_response(t_client& client);
    void handle_get(t_client& client);
    void serve_file(t_client& client, const std::string& file_path, int code);
    bool get_index(t_client& client);
    void generate_index(t_client& client, const std::string& file_path); 
    void handle_post(t_client& client);
    void handle_delete(t_client& client);
    void queue_error(t_client& client, int code);
    void queue_redirect(t_client& client,const std::string& location, int code);
    void choose_server(t_client& client);
    std::string reason_sentence(int code);
    bool parse_request(t_request &req, const std::string &buffer);
};



std::vector<std::string> ft_split(const std::string & str, const std::string &dilm);
bool	ft_method(t_request & req, const std::string &buffer);
bool	ft_headers(t_request & req, std::string & s);
std::map<std::string, std::string> ft_cookies(const std::string s);
std::string	ft_trim(std::string s, char c);


#endif
