cmake_minimum_required(VERSION 3.13.4)
project(e_curtain_cxx)

set(CMAKE_CXX_STANDARD 17)

add_executable(e_curtain_cxx main.cpp filter.cpp filter.hpp common.hpp udp_client.hpp si7021.hpp si7021.cpp)