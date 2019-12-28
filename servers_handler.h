#ifndef SERVERS_HANDLER_H
#define SERVERS_HANDLER_H

#include <functional>
#include <memory>
#include <map>

#include <sys/eventfd.h>
#include <sys/timerfd.h>
#include <sys/epoll.h>

#include "wrappers/epoll_wrapper.h"
#include "utils/server_exception.h"
#include "wrappers/file_descriptor.h"
#include "wrappers/writing_client.h"

struct server_uniq_fd;

struct servers_handler {
    epoll_wrapper epoll_;
    std::map<int, std::unique_ptr<file_descriptor>> all_connections;
    
    servers_handler();
    
    void add_server(server_uniq_fd *, uint32_t);

    void remove_server(server_uniq_fd *);
    
    void exec();
};

struct server_uniq_fd : public writing_client {
    servers_handler *parent;
    std::function<void(void)> callback;
    
    server_uniq_fd(writing_client &&, servers_handler *, std::function<void(void)>, uint32_t msk = EPOLLIN);

    virtual ~server_uniq_fd();
};

#endif //SERVERS_HANDLER_H
