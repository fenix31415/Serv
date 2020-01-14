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
#include "wrappers/writing_client.h"

struct server_uniq_fd;

struct servers_handler {
    friend struct server_uniq_fd;
    servers_handler();
    ~servers_handler();
    void exec();

private:
    inline static volatile bool executing = true;
    epoll_wrapper epoll_;

    std::unique_ptr<server_uniq_fd> signal_catcher;
    std::map<int, std::unique_ptr<file_descriptor>> all_connections;

    void add_socket(server_uniq_fd *, uint32_t);
    void remove_socket(server_uniq_fd *);
};

struct server_uniq_fd : public writing_client {
    friend struct servers_handler;

    server_uniq_fd(int, servers_handler *, std::function<void(void)>, uint32_t msk = EPOLLIN);
    virtual ~server_uniq_fd();
private:
    servers_handler *parent;
    std::function<void(void)> callback;
};

#endif //SERVERS_HANDLER_H
