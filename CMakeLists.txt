#cmake_minimum_required(VERSION 3.21)
project(Omok C)

set(CMAKE_C_STANDARD 11)

add_executable(Server Server.c)
add_executable(Client Client.c)
