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


enum ClientState
{
    READING_HEADERS,
    READING_BODY,
    SENDING,
    RUNNING_CGI,
    ROUTING,
    DONE
};



typedef struct s_request
{
    std::string method;
    std::string target;
    std::string version;
    std::map<std::string, std::string> cookies;
    std::map<std::string, std::string> headers;
    std::string body;
    size_t content_length;
    size_t body_received;

}t_request;

typedef struct s_client
{
    int fd;
    enum ClientState state;
    std::string inbuf;
    std::string outbuf;
    t_request request;
    t_config *server;
    t_location *location;

}t_client;


struct s_CgiProcess
{
    int pid;
    int in_fd;              
    int out_fd;              
    size_t to_child;    // bytes still to write
    size_t from_child;         // bytes read so far
    int client_fd;
    //started_at
}t_CgiProcess;



class server
{
private:
    std::map <int, std::vector<t_config> > servers;
    std::vector<struct pollfd> listeners;
    std::map<std::pair<std::string, size_t> , int> know_exist;
    std::map<int, t_client> clients;

public:
    server();
    ~server();
    void init(std::vector<t_config>& list);
    void run();
    void accept_new_clients(int listener_fd);
    void handle_client_read(int fd);
    void handle_client_write(int fd);
    void cleaning_client(int fd);
};





#endif