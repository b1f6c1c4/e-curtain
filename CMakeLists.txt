cmake_minimum_required(VERSION 3.13.4)
project(e_curtain_cxx)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

if(DEFINED ENV{TOOLCHAIN})
    set(CMAKE_SYSTEM_NAME Linux)
    set(CMAKE_SYSTEM_PROCESSOR armv6l)
    set(CMAKE_C_COMPILER $ENV{TOOLCHAIN}/bin/armv6-rpi-linux-gnueabihf-gcc)
    set(CMAKE_CXX_COMPILER $ENV{TOOLCHAIN}/bin/armv6-rpi-linux-gnueabihf-g++)
    set(CMAKE_SYSROOT $ENV{TOOLCHAIN}/armv6-rpi-linux-gnueabihf/sysroot)
    set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
    set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
endif()

include_directories(matlab)

file(GLOB LIBDUMBAC_SRC libdumbac/*.c)
add_library(dumbac STATIC ${LIBDUMBAC_SRC})

add_executable(controller-12 main.cpp common.hpp udp_client.hpp sync.hpp filter.cpp filter.hpp si7021.cpp si7021.hpp)

add_executable(controller-0 controller-0.cpp common.hpp udp_server.hpp sync.hpp filter.cpp filter.hpp mpc.cpp mpc.hpp debouncer.hpp rf.cpp rf.hpp)

target_link_libraries(controller-0 Threads::Threads)
target_link_libraries(controller-0 dumbac)
