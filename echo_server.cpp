#include "echo_server.h"

#include <unistd.h>
#include <sys/eventfd.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <csignal>

int create_timer_fd(int waitafter, int wait) {
    struct itimerspec tc{};
    int timerfd;

    tc.it_value.tv_sec = waitafter / 1000;
    tc.it_value.tv_nsec = waitafter % 1000;
    tc.it_interval.tv_sec = wait / 1000;
    tc.it_interval.tv_nsec = wait % 1000;

    timerfd = timerfd_create(CLOCK_MONOTONIC, 0);
    if (timerfd == -1)
        throw server_exception("timerfd_create system error");

    if (timerfd_settime(timerfd, 0, &tc, NULL) == -1)
        throw server_exception("timerfd_settime system error");
    return timerfd;
}

std::string get_addr_info(const std::string &h) {
    struct addrinfo *result;
    struct addrinfo *res;
    std::set<std::string> answer;
    utils::ensure(getaddrinfo(h.c_str(), NULL, NULL, &result), utils::is_zero, "getaddrinfo succ");

    for (res = result; res != NULL; res = res->ai_next) {
        unsigned int t = reinterpret_cast<sockaddr_in *>(res->ai_addr)->sin_addr.s_addr;
        answer.insert(inet_ntoa(in_addr{t}));
    }

    freeaddrinfo(result);

    std::string ans;
    for (const auto &i : answer)
        ans += i + "\n";
    ans += "\n";

    return ans;
}

echo_server::echo_server(servers_handler* parent, uint16_t port, bool ch) : should_check_ports(ch), port(port), parent(parent),
        server_fd(socket(AF_INET, SOCK_STREAM, 0), parent, [this]() { this->sock_handle(); }, EPOLLIN) {
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = htonl(0);

    if(!should_check_ports) {
        utils::ensure(bind(server_fd.get_fd(), (sockaddr *) &addr, sizeof(sockaddr)),
                      utils::is_zero, "Binded to port " + std::to_string(port));
    } else {
        while(bind(server_fd.get_fd(), (sockaddr *) &addr, sizeof(sockaddr)) != 0) {
            utils::ensure("[WARN]\tport "+std::to_string(port)+" (still) used, lets try port="+std::to_string(port + 1));
            ++port;
            addr.sin_family = AF_INET;
            addr.sin_port = htons(port);
            addr.sin_addr.s_addr = htonl(0);
        }
    }

    utils::ensure(0, utils::is_zero, "Binded to port " + std::to_string(port));

    utils::ensure(listen(server_fd.get_fd(), max_clients), utils::is_zero,
                  "Socket " + std::to_string(server_fd.get_fd()) + " is in listening state");

    timer = std::make_unique<server_uniq_fd>(create_timer_fd(default_timeout_after, default_timeout), parent, [this]() {
        update_connections();
        read(timer.get()->get_fd(), nullptr, 8);
        return;
    });
    utils::ensure(0, utils::is_zero, "Timer " + std::to_string(timer.get()->get_fd()) + " in server " + std::to_string(server_fd.get_fd()) + " created");

    worker = std::make_unique<std::thread>([this]() {
        while (WORKS.load()) {
            std::pair<echo_connection *, std::string> arg;
            {
                std::unique_lock<std::mutex> lg(work_in);
                cv_w.wait(lg, [this] {
                    return !jobs.empty() || !WORKS.load();
                });
                if (!WORKS.load())
                    return;
                if (!jobs.empty()) {
                    arg = jobs.front();
                    jobs.pop();
                }
            }
            try {
                arg.second = get_addr_info(arg.second);
            } catch (const server_exception &e) {
                arg.second = "Server Internal Error\n\n";
            }
            {
                std::lock_guard<std::mutex> lg(work_out);
                results.push(arg);
                cv_a.notify_one();
            }
        }
    });

    answer = std::make_unique<std::thread>([this]() {
        while (WORKS.load()) {
            {
                std::unique_lock<std::mutex> lg(work_out);
                cv_a.wait(lg, [this] {
                    return !results.empty() || !WORKS.load();
                });
                if (!WORKS.load())
                    return;
                while (!results.empty()) {
                    if (!WORKS.load())
                        return;
                    echo_connection *t = results.front().first;
                    if (connections.find(t->sock.get_fd()) != connections.end()) {
                        connections[t->sock.get_fd()]->sock.write_c(results.front().second.c_str(), results.front().second.size());
                    }
                    results.pop();
                }
            }
        }
    });

    utils::ensure(0, utils::is_zero, "Server " + std::to_string(server_fd.get_fd()) + " created.\n");
}

void echo_server::update_connections() {
    for(auto& con : connections) {
        if(con.second.get()->sock.isAlive()) {
            con.second.get()->sock.markSleeping();
        } else {
            con.second.get()->disconnect();
        }
    }
}

echo_server::~echo_server() {
    utils::ensure("Server "+std::to_string(server_fd.get_fd())+" destructor called");

    WORKS.store(false);
    {
        std::lock_guard<std::mutex> lg(work_in);
        kill(worker->native_handle(), SIGTERM);
        cv_w.notify_all();
    }
    worker->join();
    {
        std::lock_guard<std::mutex> lg(work_out);
        kill(answer->native_handle(), SIGTERM);
        cv_a.notify_all();
    }
    answer->join();

    utils::ensure("Server "+std::to_string(server_fd.get_fd())+" backgraund threads joined");
}

void echo_server::sock_handle() {
    int fd = server_fd.get_fd();
    int conn_fd = accept(fd, nullptr, nullptr);
    
    utils::ensure(conn_fd, utils::is_not_negative,
                              "New client connecting: " + std::to_string(conn_fd));

    unique_ptr<echo_connection> connect = std::make_unique<echo_connection>(this, conn_fd);
    connections.insert(std::make_pair(connect.get()->sock.get_fd(), std::move(connect)));
}

void echo_connection::sock_handle() {
    if (sock.read_from_client(BUFSIZE) == 0) {
        utils::ensure("Client " + std::to_string(sock.get_fd()) + " disconnected from the server");
        disconnect();
        return;
    }

    size_t pos = 0;
    std::string req = "";
    while(pos < sock.get_filled()) {
        char curc = sock.at(pos);
        if(curc == '\n' || curc == '\r' || curc == owner->delim_char) {
            std::lock_guard<std::mutex> lg(owner->work_in);
            if(!req.empty()) {
                owner->jobs.push(std::make_pair(this, req));
                owner->cv_w.notify_one();
            }
            sock.shl(pos + 1);
            pos = 0;
            req.clear();
            continue;
        }

        req.push_back(curc);
        ++pos;
    }
}

echo_connection::echo_connection(echo_server *owner, int fd)
        : sock(fd, owner->parent, [this]() { this->sock_handle(); }, EPOLLIN),
          stamp(time(NULL)),
            owner(owner) {
    utils::ensure(0, utils::is_zero, "Client " + std::to_string(sock.get_fd()) + " connected\n");
}

void echo_connection::disconnect() {
    owner->connections.erase(sock.get_fd());
}

echo_connection::~echo_connection() {}
