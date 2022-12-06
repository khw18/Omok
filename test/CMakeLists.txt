cmake_minimum_required(VERSION 3.21)
project(test C)

set(CMAKE_C_STANDARD 99)

add_executable(PentaGo-Server PentaGo-Server.c)
add_executable(PentaGo-Client PentaGo-Client.c)
