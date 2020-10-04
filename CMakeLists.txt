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

add_executable(F top/F.cpp)
add_executable(C top/C.cpp)
add_executable(G0 top/G0.cpp)
add_executable(G1 top/G1.cpp)
add_executable(H0 top/H0.cpp)
add_executable(H1 top/H12.cpp)
add_executable(H2 top/H12.cpp)

target_compile_definitions(H1 PRIVATE TAG=1)
target_compile_definitions(H2 PRIVATE TAG=2)

target_link_libraries(C dumbac)

target_link_libraries(F middleware)
target_link_libraries(C middleware)
target_link_libraries(G0 middleware)
target_link_libraries(G1 middleware)
target_link_libraries(H0 middleware)
target_link_libraries(H1 middleware)
target_link_libraries(H2 middleware)

target_link_libraries(F Threads::Threads)
target_link_libraries(C Threads::Threads)
target_link_libraries(G0 Threads::Threads)
target_link_libraries(G1 Threads::Threads)
target_link_libraries(H0 Threads::Threads)
target_link_libraries(H1 Threads::Threads)
target_link_libraries(H2 Threads::Threads)
