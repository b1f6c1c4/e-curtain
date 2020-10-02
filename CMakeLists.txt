cmake_minimum_required(VERSION 3.13.4)
project(e_curtain_cxx)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)
find_library(libatomic NAMES atomic)

if(DEFINED ENV{TOOLCHAIN})
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR armv6l)
    set(CMAKE_C_COMPILER $ENV{TOOLCHAIN}/bin/armv6l-unknown-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER $ENV{TOOLCHAIN}/bin/armv6l-unknown-linux-gnueabihf-g++)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
endif()

include_directories(matlab libdumbac)

file(GLOB LIBDUMBAC_SRC libdumbac/*.c)
add_library(libdumbac STATIC ${LIBDUMBAC_SRC})

add_executable(controller-12 main.cpp common.hpp udp_client.hpp sync.hpp filter.cpp filter.hpp si7021.cpp si7021.hpp)

add_executable(controller-0 controller-0.cpp common.hpp udp_server.hpp sync.hpp filter.cpp filter.hpp mpc.cpp mpc.hpp)

target_link_libraries(controller-0 Threads::Threads)
target_link_libraries(controller-0 ${libatomic})
target_link_libraries(controller-0 libdumbac)
