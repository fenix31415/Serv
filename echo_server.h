#ifndef SERVER_ECHO_SERVER_H_H
#define SERVER_ECHO_SERVER_H_H

#include <map>
#include <set>
#include <queue>
#include <atomic>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "wrappers/writing_client.h"
#include "servers_handler.h"

using std::unique_ptr;

struct echo_connection;

struct echo_server {
friend struct echo_connection;
private:
    static const char delim_char = '^';
    static const uint16_t max_clients = 8;
    static const int default_timeout_after = 10000; //ms
    static const int default_timeout = 100000; //ms
    const bool should_check_ports;

    uint16_t port;
    servers_handler *parent;
    server_uniq_fd server_fd;

    unique_ptr<server_uniq_fd> timer;
    std::map<int, unique_ptr<echo_connection>> connections;
    std::set<echo_connection*> deleted_connections;


    static inline volatile std::atomic_bool WORKS = true;
    std::queue<std::pair<echo_connection *, std::string>> jobs;
    std::queue<std::pair<echo_connection *, std::string>> results;
    unique_ptr<std::thread> worker;
    unique_ptr<std::thread> answer;
    std::mutex work_in, work_out;
    std::condition_variable cv_w;
    std::condition_variable cv_a;


    void sock_handle();
    void update_connections();

public:
    echo_server(servers_handler*, uint16_t, bool = true);
    ~echo_server();

    echo_server(echo_server const&) = delete;
    echo_server& operator=(echo_server const&) = delete;
};

struct echo_connection {
    friend struct echo_server;
public:
    echo_connection(echo_server*, int);
    ~echo_connection();
private:
    inline static const size_t BUFSIZE = 32;
    char buffer[BUFSIZE];

    echo_server* owner;
    server_uniq_fd sock;
    const time_t stamp;

    void sock_handle();
    void disconnect();
};

#endif //SERVER_ECHO_SERVER_H_H
