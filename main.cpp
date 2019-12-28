#include "utils/server_exception.h"
#include <iostream>
#include <map>
#include "echo_server.h"

int main() {
    try {
        servers_handler proc;
        std::cout<<"proc created\n";
        echo_server server1(&proc, 1504);
        std::cout<<"1 created\n";
        echo_server server2(&proc, 1505);
        std::cout<<"2 created\n";
        
        std::cout<<"starting..\n";
        proc.exec();
    } catch (server_exception const& e) {
        std::cerr << e.what() << std::endl;
    } catch (...) {
        std::cout<<"..\n";
    }
}
