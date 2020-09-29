cmake_minimum_required(VERSION 3.16)
project(e_curtain_cxx)

set(CMAKE_CXX_STANDARD 17)

add_executable(e_curtain_cxx main.cpp filter.cpp filter.hpp common.h udp_client.hpp)