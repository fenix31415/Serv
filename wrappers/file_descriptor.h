#ifndef SERVER_FD_H
#define SERVER_FD_H

#include <functional>

#include "../utils/util_functions.h"

struct file_descriptor {
public:
    explicit file_descriptor(int);
    virtual ~file_descriptor();

    file_descriptor(file_descriptor const &) = delete;
    file_descriptor operator=(file_descriptor const &) = delete;

    file_descriptor(file_descriptor &&) noexcept;

    explicit file_descriptor(std::function<int(void)> &&);

    int get_fd() const;

protected:
    int fd_val;
};


#endif //SERVER_FD_H
