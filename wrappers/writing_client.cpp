#include "writing_client.h"

#include <cstring>
#include <unistd.h>

writing_client::writing_client(int fd) : file_descriptor(fd), st_buffer(BUFSIZ) {
}

ssize_t writing_client::read_from_client(size_t count) {
    ssize_t ret = read_tobuf(count);
    markWokedUp();
    return ret;
}

char writing_client::at(size_t i) {
    return st_buffer.buffer[i];
}

void writing_client::shl(size_t val) {
    return st_buffer.shl(val);
}

ssize_t writing_client::read_c(void *src, size_t len) {
    return read(fd_val, src, len);
}

writing_client::~writing_client() {}

writing_client::storing_buffer::~storing_buffer() {
    delete[] buffer;
}

void writing_client::storing_buffer::shl(size_t val) {
    filled -= val;
    std::memmove(buffer, buffer + val, filled);
}

size_t writing_client::get_filled() {
    return st_buffer.filled;
}

bool writing_client::isAlive() {
    return alive;
}

void writing_client::markSleeping() {
    alive = false;
}

void writing_client::markWokedUp() {
    alive = true;
}

ssize_t writing_client::read_tobuf(size_t count) {
    ssize_t r = read_c((void *) (st_buffer.buffer + get_filled()), count);
    utils::ensure(r, utils::is_not_negative,
                  r != 0 ? std::to_string(r) + " bytes read and put in buffer" : "");
    st_buffer.filled += r;
    return r;
}

writing_client::writing_client(writing_client &&arg) noexcept : file_descriptor(arg.fd_val), st_buffer(BUFSIZ) {
    arg.fd_val = -1;
}

ssize_t writing_client::write_c(const void *src, size_t len) {
    return write(fd_val, src, len);
}
