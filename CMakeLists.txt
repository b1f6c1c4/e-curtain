cmake_minimum_required(VERSION 3.13.4)
project(e_curtain_cxx)

set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(json GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent GIT_TAG v3.9.1)
FetchContent_GetProperties(json)
if(NOT json_POPULATED)
    FetchContent_Populate(json)
    add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

if(DEFINED ENV{TOOLCHAIN} AND NOT $ENV{TOOLCHAIN} STREQUAL "")
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

link_libraries(middleware Threads::Threads)
add_executable(H0-C top/H0-C.cpp)
add_executable(F-G0 top/F-G0.cpp)
add_executable(H1-G1 top/H1-G1.cpp)
add_executable(H2 top/H2.cpp)
add_executable(debugger top/debugger.cpp)

target_link_libraries(H0-C nlohmann_json::nlohmann_json dumbac)

