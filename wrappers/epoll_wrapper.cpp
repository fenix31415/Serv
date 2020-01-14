#include "epoll_wrapper.h"

epoll_wrapper::epoll_wrapper() : epoll_fd(file_descriptor(epoll_create1(0))) {
    utils::ensure(epoll_fd.get_fd(), utils::is_not_negative,
                  "Epoll fd created: " + std::to_string(epoll_fd.get_fd()));

    events = new epoll_event[max_events];
}

epoll_wrapper::~epoll_wrapper() {
    utils::ensure("Epoll destructor called");
    delete[] events;
}

std::pair<int, epoll_event *> epoll_wrapper::start_sleeping(int timeout) {
    int num = epoll_wait(epoll_fd.get_fd(), events, max_events, timeout);
    utils::ensure(num, utils::is_not_negative, "Epoll has waken up with " + std::to_string(num) + " new events");
    return {num, events};
}

int epoll_wrapper::get_fd() {
    return epoll_fd.get_fd();
}
