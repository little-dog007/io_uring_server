cmake_minimum_required(VERSION 3.15)
project(liburing_examples C)

set(CMAKE_C_STANDARD 99)

add_executable(webserver_liburing webserver_liburing.c)
target_link_libraries(webserver_liburing uring)
