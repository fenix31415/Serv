#ifndef SERVER_SERVER_EXCEPTIONS_H
#define SERVER_SERVER_EXCEPTIONS_H

#include <exception>
#include <string>

struct server_exception : std::exception {
    server_exception(const std::string& msg) : msg(msg) {}

    server_exception(std::string&& msg) : msg(std::move(msg)) {}

    const char *what() const noexcept override {
        return msg.c_str();
    }

    std::string get_msg() {
        return msg;
    }

private:
    std::string msg;
};

#endif //SERVER_SERVER_EXCEPTIONS_H
