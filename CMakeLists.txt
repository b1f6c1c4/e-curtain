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

include_directories(include matlab)

file(GLOB LIBDUMBAC_SRC libdumbac/*.c)
add_library(dumbac STATIC ${LIBDUMBAC_SRC})

add_executable(controller-12 controller-1.cpp include/common.hpp include/net/udp_client.hpp include/sync.hpp middleware/dsp/filter.cpp include/dsp/filter.hpp middleware/io/si7021.cpp include/io/si7021.hpp)

add_executable(controller-0 controller-0.cpp include/common.hpp include/net/udp_server.hpp include/sync.hpp middleware/dsp/filter.cpp include/dsp/filter.hpp middleware/dsp/mpc.cpp include/dsp/mpc.hpp include/dsp/debouncer.hpp middleware/io/rf.cpp include/io/rf.hpp middleware/io/buzzer.cpp include/io/buzzer.hpp)

target_link_libraries(controller-0 Threads::Threads)
target_link_libraries(controller-0 dumbac)
