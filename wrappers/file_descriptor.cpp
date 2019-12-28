#include "file_descriptor.h"
#include <unistd.h>

file_descriptor::~file_descriptor() {
    if(fd_val==-1)return;
    utils::ensure(close(fd_val), utils::is_zero,
                  "File descriptor " + std::to_string(fd_val) + " was closed\n");
}

file_descriptor::file_descriptor(int val) : fd_val(val) {
    utils::ensure(val, utils::is_not_negative,
                  "Fd " + std::to_string(val) + " wrapped\n");
}

int file_descriptor::get_fd() const { return fd_val; }

file_descriptor::file_descriptor(file_descriptor &&arg) noexcept : fd_val(arg.fd_val) { arg.fd_val = -1; }

file_descriptor::file_descriptor(function<int(void)> &&creator) { fd_val = creator(); }



