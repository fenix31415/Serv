#ifndef SERVER_UTIL_FUNCTIONS_H
#define SERVER_UTIL_FUNCTIONS_H

#include <functional>

namespace utils
{
    void ensure(int val, std::function<bool(int)> if_successful, std::string on_success);

    void ensure(std::string on_success);

    bool is_not_negative(int a);
    bool is_zero(int a);
}

#endif //SERVER_UTIL_FUNCTIONS_H
