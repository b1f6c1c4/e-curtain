cmake_minimum_required(VERSION 3.13.4)
project(e_curtain_cxx)

set(CMAKE_CXX_STANDARD 17)

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

add_executable(controller-12 main.cpp filter.cpp filter.hpp common.hpp udp_client.hpp si7021.hpp si7021.cpp mpc.cpp mpc.hpp)

include_directories(gen gen-include)
add_executable(controller-0 controller-0.cpp common.hpp udp_server.hpp
        gen/rt_nonfinite.c gen/rtGetNaN.c gen/rtGetInf.c gen/mpcmoveCodeGeneration_data.c
        gen/mpcmoveCodeGeneration_initialize.c gen/mpcmoveCodeGeneration_terminate.c gen/mpcmoveCodeGeneration.c
        gen/mpcblock_optimizer.c gen/qpkwik.c gen/minOrMax.c gen/qr.c gen/xnrm2.c gen/xgerc.c gen/norm.c)
