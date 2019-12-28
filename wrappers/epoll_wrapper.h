#ifndef SERVER_EPOLL_WRAPPER_H
#define SERVER_EPOLL_WRAPPER_H

#include <memory>
#include <map>
#include <sys/epoll.h>
#include "file_descriptor.h"
#include <experimental/optional>

struct epoll_wrapper {
private:
    std::experimental::optional<file_descriptor> signal_fd;

    epoll_event *events;

public:
    static const uint16_t default_max_epoll_events = 1024;
    file_descriptor epoll_fd;
    ~epoll_wrapper();

    int get_signal_fd();

    epoll_wrapper();

    void add_server(file_descriptor* server_fd);

    std::pair<int, epoll_event *> start_sleeping(int timeout);

    void add_client(file_descriptor *client);

    void remove_client(file_descriptor *client);

    void add_signal_handling();
};


#endif //SERVER_EPOLL_WRAPPER_H
