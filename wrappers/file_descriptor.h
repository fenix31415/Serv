#ifndef SERVER_FD_H
#define SERVER_FD_H

#include <functional>

#include "../utils/util_functions.h"

using std::function;

struct file_descriptor {
protected:
    int fd_val;
public:
    int get_fd() const;
    explicit file_descriptor(int);
    virtual ~file_descriptor();

    file_descriptor(file_descriptor const &) = delete;

    file_descriptor operator=(file_descriptor const &) = delete;

    file_descriptor(file_descriptor &&) noexcept;

    explicit file_descriptor(function<int(void)> &&);

};


#endif //SERVER_FD_H
