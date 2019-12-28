#include "writing_client.h"
#include <cstring>
#include <unistd.h>

writing_client::writing_client(int fd) : file_descriptor(fd), st_buffer(BUFSIZ) {}

ssize_t writing_client::read_from_client(size_t count)
{
    ssize_t ret = abstract_read(count);
    //it = dc.update(it, std::time(nullptr) + timeout);
    return ret;
}

void writing_client::write_to_client()
{
    abstract_write();
    //it = dc.update(it, std::time(nullptr) + timeout);
}

writing_client::~writing_client() {}

writing_client::storing_buffer::~storing_buffer()
{
    delete[] buffer;
}

void writing_client::storing_buffer::shl(size_t val)
{
    filled -= val;
    std::memmove(buffer, buffer + val, filled);
}

size_t writing_client::get_filled()
{
    return st_buffer.filled;
}

bool writing_client::is_buffer_empty()
{
    return this->get_filled() == 0;
}

ssize_t writing_client::abstract_read(size_t count)
{
    ssize_t r = read(get_fd(), (void *) (this->st_buffer.buffer + this->get_filled()),
                     count);
    this->st_buffer.filled += r;
    utils::ensure(r, utils::is_not_negative,
                  r != 0 ? std::to_string(r) + " bytes read and put in buffer\n" : "");
    return r;
}

void writing_client::abstract_write()
{
    ssize_t w = write(get_fd(), (void *) this->st_buffer.buffer, this->get_filled());
    utils::ensure(w, utils::is_not_negative, "");
    this->st_buffer.shl((size_t) w); //casting is ok, 'cause non negative

    utils::ensure(std::to_string(w) + " bytes written, " + std::to_string(this->get_filled())
                  + " left in buffer of client " + std::to_string(get_fd()) + "\n");
}

writing_client::writing_client(writing_client &&arg) noexcept : file_descriptor(arg.fd_val), st_buffer(BUFSIZ) { arg.fd_val = -1; }

