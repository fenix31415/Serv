#include "servers_handler.h"
#include "utils/util_functions.h"

#include<set>
#include <signal.h>
#include <sys/signalfd.h>

int create_signal_fd() {
    sigset_t mask;
    int sfd;

    sigemptyset(&mask);
    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGQUIT);

    utils::ensure(sigprocmask(SIG_BLOCK, &mask, NULL), utils::is_not_negative, "Signalls INT&QUIT blocked");

    sfd = signalfd(-1, &mask, 0);
    utils::ensure(sfd, utils::is_not_negative, "Signals fd is " + std::to_string(sfd));
    return sfd;
}

servers_handler::~servers_handler() {
    utils::ensure("Handler destructor called");
}

servers_handler::servers_handler() {
    utils::ensure(0, utils::is_zero, "Start creating Handler");
    executing = true;

    signal_catcher = std::make_unique<server_uniq_fd>(create_signal_fd(), this, [this]() {
        struct signalfd_siginfo fdsi{};
        ssize_t s = this->signal_catcher->read_c(&fdsi, sizeof(struct signalfd_siginfo));
        if (s != sizeof(struct signalfd_siginfo))
            return;
        if (fdsi.ssi_signo == SIGINT) {
            this->executing = false;
        } else if (fdsi.ssi_signo == SIGQUIT) {
            exit(EXIT_SUCCESS);
        } else {
            exit(EXIT_FAILURE);
        }
        return;
    });

    utils::ensure(0, utils::is_zero, "Handler created\n");
}

void servers_handler::exec() {
    executing = true;
    while (executing) {
        std::pair<int, epoll_event *> res = epoll_.start_sleeping(-1);
        if (res.first == -1) {
            throw server_exception("-1 events..");
        } else {
            for (int i = 0; i < res.first; i++) {
                reinterpret_cast<server_uniq_fd *> (res.second[i].data.ptr)->callback();
            }
        }
    }
}

void servers_handler::add_socket(server_uniq_fd *sock, uint32_t msk) {
    epoll_event ev{};
    ev.events = msk;
    ev.data.fd = sock->get_fd();
    ev.data.ptr = reinterpret_cast<void *>(sock);
    utils::ensure(epoll_ctl(epoll_.get_fd(), EPOLL_CTL_ADD, sock->get_fd(), &ev),
                  utils::is_zero, "Socket " + std::to_string(sock->get_fd()) + " added to epoll");
}

void servers_handler::remove_socket(server_uniq_fd *sock) {
    utils::ensure(epoll_ctl(epoll_.get_fd(), EPOLL_CTL_DEL, sock->get_fd(), NULL),
                  utils::is_zero, "Socket " + std::to_string(sock->get_fd()) + " removed from epoll");

}

server_uniq_fd::server_uniq_fd(int fd, servers_handler *proc, std::function<void(void)> callback, uint32_t msk)
        : writing_client(fd), parent(proc), callback(std::move(callback)) {
    parent->add_socket(this, msk);
}

server_uniq_fd::~server_uniq_fd() {
    utils::ensure("Socket "+std::to_string(get_fd())+" destructor called");
    parent->remove_socket(this);
}
