#include "servers_handler.h"

#include<set>

servers_handler::servers_handler() {
    epoll_.add_signal_handling();
}

void servers_handler::exec() {
    while (true) {
        //int ready = epoll_wait(epoll_fd.get_fd(), pevents, epoll_.default_max_epoll_events, EPOLL_TIMEOUT);
        std::pair<int, epoll_event *> res = epoll_.start_sleeping(-1);
        if (res.first == -1) {
            throw server_exception("-1 events..");
        } else {
            for (int i = 0; i < res.first; i++) {
                if(res.second[i].data.fd == epoll_.get_signal_fd()) {
                    utils::ensure("Server is closing because of a signal.\n");
                    throw server_exception("Caught signal");
                }
                
                //utils::ensure("starting callback " + std::to_string(res.second[i].data.fd) + "\n");
                reinterpret_cast<server_uniq_fd *> (res.second[i].data.ptr)->callback();
                //utils::ensure("finished callback\n");
            }
        }
    }
}

void servers_handler::add_server(server_uniq_fd *sock, uint32_t msk) {
    epoll_event ev{};
    ev.events = msk;
    ev.data.fd = sock->get_fd();
    ev.data.ptr = reinterpret_cast<void *>(sock);
    utils::ensure(epoll_ctl(epoll_.epoll_fd.get_fd(), EPOLL_CTL_ADD, sock->get_fd(), &ev),
                  utils::is_zero, "Server " + std::to_string(sock->get_fd()) + " added\n");
    //if (epoll_ctl(epoll_.epoll_fd.get_fd(), EPOLL_CTL_ADD, sock->get_fd(), &ev) != 0) {
    //    throw server_exception("epoll add");
    //}
}

void servers_handler::remove_server(server_uniq_fd *sock) {
    //if (epoll_ctl(epoll_.epoll_fd.get_fd(), EPOLL_CTL_DEL, sock->get_fd(), NULL) != 0) {
    //    throw server_exception("epoll remove");
    //}
    utils::ensure(epoll_ctl(epoll_.epoll_fd.get_fd(), EPOLL_CTL_DEL, sock->get_fd(), NULL),
                  utils::is_zero, "Server " + std::to_string(epoll_.epoll_fd.get_fd()) + " removed\n");
}

server_uniq_fd::server_uniq_fd(writing_client &&fd, servers_handler *proc, std::function<void(void)> callback, uint32_t msk)
        : writing_client(std::move(fd)), parent(proc), callback(std::move(callback)) {
    parent->add_server(this, msk);
}

server_uniq_fd::~server_uniq_fd() {
    parent->remove_server(this);
}
