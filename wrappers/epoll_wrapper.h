#ifndef SERVER_EPOLL_WRAPPER_H
#define SERVER_EPOLL_WRAPPER_H

#include <sys/epoll.h>

#include "file_descriptor.h"

struct epoll_wrapper {
    epoll_wrapper();
    ~epoll_wrapper();

    int get_fd();

    std::pair<int, epoll_event *> start_sleeping(int);
private:
    static const uint16_t max_events = 1024;

    epoll_event *events;
    file_descriptor epoll_fd;
};

#endif //SERVER_EPOLL_WRAPPER_H
