#include <cstring>
#include <iostream>

#include "util_functions.h"
#include "server_exception.h"

bool ::utils::is_not_negative(int a) {
    return a >= 0;
}
bool ::utils::is_zero(int a) {
    return a == 0;
}

void ::utils::ensure(int val, std::function<bool(int)> if_successful, std::string on_success) {
    if (!if_successful(val)) {
        std::cerr<<"[!!!] ("<<errno<<") utils check fail: " << std::string(strerror(errno)) << std::endl;
        std::cerr<<"NOT "<<on_success<<std::endl;
        throw server_exception("Check fall: " + std::string(strerror(errno))+"\n");
    } else {
        if (on_success != "")
            std::cerr << "[ok]\t" << on_success << std::endl;
    }
}

void ::utils::ensure(std::string str) {
    std::cerr << "[!!]"<<str << std::endl;
}
