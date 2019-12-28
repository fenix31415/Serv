#ifndef SERVER_ECHO_SERVER_H_H
#define SERVER_ECHO_SERVER_H_H

#include "wrappers/writing_client.h"
#include "utils/server_exception.h"
#include "utils/util_functions.h"
#include "servers_handler.h"

#include <cstdint>
#include <map>
#include <set>

using std::map;
using std::set;
using std::unique_ptr;

struct echo_connection;
struct servers_handler;

struct echo_server {
friend struct echo_connection;
private:
    static const uint16_t default_port = 8667;
    static const uint16_t default_max_clients = 16;
    const size_t default_client_buffer_size = 64;
    const size_t default_buffer_size = 16;
    const int default_timeout = 10;
    void clean_old_connections();
    unique_ptr<server_uniq_fd> timer;
    uint16_t port;
    server_uniq_fd server_fd;
    
    //std::map<echo_connection*, unique_ptr<echo_connection>> connections;
    std::map<int, unique_ptr<echo_connection>> connections;
    set<echo_connection*> deleted_connections;
    //std::map<int, std::shared_ptr<echo_client>> all_clients;
    //deadline_container dc;

    void start_listening();
    void sock_handle();
    void add_connection(int fd);

public:
    servers_handler *parent;

    echo_server(servers_handler* parent, uint16_t port);

    ~echo_server();

    echo_server(echo_server const&) = delete;
    echo_server& operator=(echo_server const&) = delete;
};

struct echo_connection {
    friend struct echo_server;
private:
    echo_server* owner;
    server_uniq_fd sock;
    void sock_handle();
    const time_t stamp;

    void disconnect();
public:
    echo_connection(echo_server*, writing_client&&);
    ~echo_connection();

private:
    inline static const size_t BUFSIZE = 32;

    char buffer[BUFSIZE];
    size_t pos = 0;
    size_t len = 0;
};

#endif //SERVER_ECHO_SERVER_H_H
