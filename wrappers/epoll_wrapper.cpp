#include "../utils/util_functions.h"

#include "epoll_wrapper.h"
#include "../utils/server_exception.h"

#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <cstring>

epoll_wrapper::epoll_wrapper() : epoll_fd(file_descriptor(epoll_create1(0))) {
    utils::ensure(epoll_fd.get_fd(), utils::is_not_negative,
                  "Epoll fd created: " + std::to_string(epoll_fd.get_fd()) + "\n");

    events = new epoll_event[default_max_epoll_events];
}

epoll_wrapper::~epoll_wrapper() {
    utils::ensure("Epoll destructor called\n");
    delete[] events;
}

int epoll_wrapper::get_signal_fd() {
    return signal_fd->get_fd();
}

void epoll_wrapper::add_server(file_descriptor* server_fd) {
    epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd->get_fd();
    event.data.ptr = reinterpret_cast<void *>(server_fd);
    utils::ensure(epoll_ctl(epoll_fd.get_fd(), EPOLL_CTL_ADD, server_fd->get_fd(), &event), utils::is_zero,
                  "Listening socket " + std::to_string(server_fd->get_fd()) + " added to epoll " +
                  std::to_string(epoll_fd.get_fd()) + "\n");
}

void epoll_wrapper::add_client(file_descriptor *client) {
    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN | EPOLLOUT | EPOLLET;
    event.data.fd = client->get_fd();

    utils::ensure(epoll_ctl(epoll_fd.get_fd(), EPOLL_CTL_ADD, client->get_fd(), &event),
                  utils::is_zero,
                  "Client " + std::to_string(client->get_fd()) + " added to epoll " +
                  std::to_string(epoll_fd.get_fd()) + "\n");
}

void epoll_wrapper::add_signal_handling() {
    if (signal_fd) {
        throw server_exception("signals already set up");
    }

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);

    utils::ensure(sigprocmask(SIG_BLOCK, &mask, NULL), utils::is_not_negative,
                  "Default handlers for signals disabled\n");

    signal_fd.emplace(signalfd(-1, &mask, 0));
    utils::ensure(signal_fd->get_fd(), utils::is_not_negative,
                  "Signal fd " + std::to_string(signal_fd->get_fd()) + " created\n");

    epoll_event event;
    memset(&event, 0, sizeof(epoll_event));
    event.events = EPOLLIN;
    event.data.fd = signal_fd->get_fd();
    utils::ensure(epoll_ctl(epoll_fd.get_fd(), EPOLL_CTL_ADD, signal_fd->get_fd(), &event),
                  utils::is_zero, "Signalfd added to epoll\n");
}

std::pair<int, epoll_event *> epoll_wrapper::start_sleeping(int timeout) {
    utils::ensure(0, utils::is_not_negative,
                  "Epoll " + std::to_string(epoll_fd.get_fd()) + " is going to sleep with timeout (secs): " +
                  std::to_string(timeout) + "\n");


    int num = epoll_wait(epoll_fd.get_fd(), events, default_max_epoll_events, -1);

    utils::ensure(num, utils::is_not_negative, "Epoll has waken up with " + std::to_string(num) + " new events" +
                                               (num == 0 ? ". Timeout exceeded" : "") + "\n");
    return {num, events};
}

void epoll_wrapper::remove_client(file_descriptor *client) {
    utils::ensure(epoll_ctl(epoll_fd.get_fd(), EPOLL_CTL_DEL, client->get_fd(), nullptr),
                  utils::is_zero, "Client " + std::to_string(client->get_fd()) + " removed from epoll\n");
}
