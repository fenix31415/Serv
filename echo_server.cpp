#include "echo_server.h"

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/eventfd.h>
#include <iostream>

echo_server::echo_server(servers_handler* parent, uint16_t port) : port(port), parent(parent),
        server_fd(writing_client(socket(AF_INET, SOCK_STREAM, 0)), parent, [this]() { this->sock_handle(); }, EPOLLIN) {
    utils::ensure(server_fd.get_fd(), utils::is_not_negative,
                  "Server fd created " + std::to_string(server_fd.get_fd()) + "\n");

    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(0);
    utils::ensure(bind(server_fd.get_fd(), (sockaddr *) &addr, sizeof(sockaddr)),
                  utils::is_zero, "Binded to port " + std::to_string(port) + "\n");
    start_listening();


    int msec = 10000;
    struct itimerspec tc{};
    int timerfd;

    tc.it_value.tv_sec = msec / 1000;
    tc.it_value.tv_nsec = 0;
    tc.it_interval.tv_sec = 10;
    tc.it_interval.tv_nsec = 0;

    timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd == -1)
        throw server_exception("timerfd_create system error");

    if (timerfd_settime(timerfd, 0, &tc, NULL) == -1)
        throw server_exception("timerfd_settime system error");

    timer = std::make_unique<server_uniq_fd>(writing_client(timerfd), parent, [this]() {
        update_connections();
        timer.get()->read_from_client(10);
        //std::cout<<"cleaned\n";
        return;
    }, EPOLLIN);
    std::cout<<"timer added\n";
}

void echo_server::update_connections() {
    for(auto& con : connections) {
        if(con.second.get()->sock.alive) {
            con.second.get()->sock.alive = false;
            //std::cout<<con.first<<"was alive\n";
        } else {
            con.second.get()->disconnect();
        }
    }
}

echo_server::~echo_server() {
    utils::ensure("Server destructor called\n");
}

void echo_server::start_listening() {
    utils::ensure(listen(server_fd.get_fd(), default_max_clients), utils::is_zero,
                  "Socket " + std::to_string(server_fd.get_fd()) + " is in listening state\n");
}

void echo_server::add_connection(int client_fd) {
    unique_ptr<echo_connection> connect = std::make_unique<echo_connection>(this, writing_client(client_fd));
    connections.insert(std::make_pair(connect.get()->sock.get_fd(), std::move(connect)));
}

void echo_server::sock_handle() {
    //when fd=myservfd
    int fd = server_fd.get_fd();
    int conn_fd = accept(fd, nullptr, nullptr);
    
    utils::ensure(conn_fd, utils::is_not_negative,
                              "New client accepted: " + std::to_string(conn_fd) + " and soon wrapped\n");
    add_connection(conn_fd);
}

void echo_connection::sock_handle() {
    if (sock.read_from_client(BUFSIZE) == 0) {
        utils::ensure("Client " + std::to_string(sock.get_fd()) + " disconnected from the server\n");

        disconnect();
        return;
    }

    sock.write_to_client();
}

echo_connection::echo_connection(echo_server *owner, writing_client &&fd)
        : sock(std::move(fd), owner->parent, [this]() { this->sock_handle(); }, EPOLLIN),
          stamp(time(NULL)),
            owner(owner) {
  utils::ensure("Got client " + std::to_string(sock.get_fd()) + "\n");
}

void echo_connection::disconnect() {
    int fd = sock.get_fd();
    owner->connections.erase(fd);
    std::cout<<"Client " << std::to_string(fd) << " disconnected from the server\n";
}

echo_connection::~echo_connection() {}
