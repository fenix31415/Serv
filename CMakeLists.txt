project(echo_server)
cmake_minimum_required(VERSION 3.5)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS)
add_executable("echo_server"
        main.cpp
        servers_handler.cpp servers_handler.h
        echo_server.cpp echo_server.h
        utils/util_functions.cpp utils/util_functions.h utils/server_exception.h
        wrappers/epoll_wrapper.cpp wrappers/epoll_wrapper.h
        wrappers/file_descriptor.cpp wrappers/file_descriptor.h
        wrappers/writing_client.cpp wrappers/writing_client.h)

