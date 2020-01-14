#ifndef SERVER_WRITING_CLIENT_H
#define SERVER_WRITING_CLIENT_H

#include "file_descriptor.h"

struct writing_client : public file_descriptor {
private:
    bool alive = true;

    struct storing_buffer {
        size_t filled;
        const size_t buffer_size;
        char *const buffer;

        storing_buffer(const size_t buffer_size) : filled(0), buffer_size(buffer_size), buffer(new char[buffer_size])
        {}

        ~storing_buffer();

        void shl(size_t);
    } st_buffer;

public:
    explicit writing_client(int);
    virtual ~writing_client();
    writing_client(writing_client &&) noexcept;

    writing_client(writing_client const &) = delete;
    writing_client operator=(writing_client const &) = delete;

    char at(size_t);
    void shl(size_t);
    ssize_t write_c(const void *, size_t);
    ssize_t read_c(void *, size_t);
    ssize_t read_from_client(size_t count);
    size_t get_filled();

    bool isAlive();
    void markSleeping();
    void markWokedUp();

private:
    ssize_t read_tobuf(size_t count);
};

#endif //SERVER_WRITING_CLIENT_H
