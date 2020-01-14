#include <unistd.h>

#include "file_descriptor.h"

file_descriptor::file_descriptor(int val) : fd_val(val) {
    utils::ensure(val, utils::is_not_negative, "Fd " + std::to_string(val) + " wrapped");
}

file_descriptor::~file_descriptor() {
    if(fd_val==-1) {
        utils::ensure("[!]\tFile descriptor ALREADY closed");
        return;
    }
    utils::ensure(close(fd_val), utils::is_zero, "File descriptor " + std::to_string(fd_val) + " was closed");
}

file_descriptor::file_descriptor(file_descriptor &&arg) noexcept : fd_val(arg.fd_val) {
    arg.fd_val = -1;
}

file_descriptor::file_descriptor(std::function<int(void)> &&creator) {
    fd_val = creator();
}

int file_descriptor::get_fd() const { return fd_val; }
