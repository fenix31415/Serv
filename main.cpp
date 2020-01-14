#include <iostream>
#include <map>
#include "echo_server.h"

int main() {
    try {
        servers_handler proc;

        echo_server server1(&proc, 1509);

        echo_server server2(&proc, 1505);

        proc.exec();
    } catch (server_exception const& e) {
        std::cerr << e.what() << std::endl;
    } catch (std::exception const& e) {
        std::cerr<<"unrecognised exception " << e.what() << std::endl;
    }
}
