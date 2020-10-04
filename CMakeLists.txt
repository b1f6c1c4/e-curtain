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

include_directories(include libdumbac matlab/extern/include matlab/rtw/c/src matlab/simulink/include)

add_library(dumbac STATIC libdumbac/libdumbac.cpp libdumbac/libdumbac_capi.cpp libdumbac/libdumbac_data.cpp)

file(GLOB MIDDLEWARE_SRC middleware/**/*.cpp)
add_library(middleware STATIC ${MIDDLEWARE_SRC})

add_executable(controller-0 controller-0.cpp include/common.hpp include/net/udp_server.hpp include/sync.hpp include/dsp/filter.hpp include/dsp/mpc.hpp include/dsp/debouncer.hpp include/io/rf.hpp include/io/buzzer.hpp)
add_executable(controller-1 controller-1.cpp include/common.hpp include/net/udp_client.hpp include/sync.hpp include/dsp/filter.hpp include/io/si7021.hpp)
add_executable(controller-2 controller-2.cpp include/common.hpp include/net/udp_client.hpp include/sync.hpp include/dsp/filter.hpp include/io/si7021.hpp)

target_link_libraries(controller-0 dumbac)

target_link_libraries(controller-0 middleware)
target_link_libraries(controller-1 middleware)
target_link_libraries(controller-2 middleware)

target_link_libraries(controller-0 Threads::Threads)
target_link_libraries(controller-1 Threads::Threads)
target_link_libraries(controller-2 Threads::Threads)
