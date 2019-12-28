#ifndef PROXY_SERVER_DEADLINE_CLIENT_H
#define PROXY_SERVER_DEADLINE_CLIENT_H

#include "file_descriptor.h"

struct writing_client : public file_descriptor {
private:
    struct storing_buffer
    {
        size_t filled;
        const size_t buffer_size;
        char *const buffer;

        storing_buffer(const size_t buffer_size) : filled(0), buffer_size(buffer_size), buffer(new char[buffer_size])
        {}


        ~storing_buffer();

        void shl(size_t val);
    };

    storing_buffer st_buffer;


    inline static const size_t BUFSIZE = 32;
public:
    ~writing_client();

    ssize_t read_from_client(size_t count);
    void write_to_client();
    writing_client(writing_client &&a) noexcept;

    explicit writing_client(int);

    writing_client(writing_client const &) = delete;

    writing_client operator=(writing_client const &) = delete;

protected:

    ssize_t abstract_read(size_t count);
    void abstract_write();

    size_t get_filled();

    bool is_buffer_empty();

};

#endif //PROXY_SERVER_DEADLINE_CLIENT_H
