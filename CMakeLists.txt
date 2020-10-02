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

include_directories(gen gen-include)

add_executable(controller-12 main.cpp common.hpp udp_client.hpp sync.hpp filter.cpp filter.hpp si7021.cpp si7021.hpp)

add_executable(controller-0 controller-0.cpp common.hpp udp_server.hpp sync.hpp filter.cpp filter.hpp mpc.cpp mpc.hpp
        gen/rt_nonfinite.c gen/rtGetNaN.c gen/rtGetInf.c gen/mpcmoveCodeGeneration_data.c
        gen/mpcmoveCodeGeneration_initialize.c gen/mpcmoveCodeGeneration_terminate.c gen/mpcmoveCodeGeneration.c
        gen/mpcblock_optimizer.c gen/mpc_calculatehessian.c gen/mpc_checkhessian.c gen/xpotrf.c gen/minOrMax.c
        gen/norm.c gen/trisolve.c gen/mpc_solveQP.c gen/qpkwik.c gen/qr.c gen/xnrm2.c gen/xgerc.c gen/xgerc.c)

target_link_libraries(controller-0 Threads::Threads)
target_link_libraries(controller-0 ${libatomic})
