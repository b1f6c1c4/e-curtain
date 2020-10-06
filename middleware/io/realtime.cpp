#include "io/realtime.hpp"
#include <iostream>
#include <pthread.h>
#include <sched.h>

void g_make_realtime() {
    auto self{ pthread_self() };
    sched_param p;
    p.sched_priority = sched_get_priority_max(SCHED_RR);
    if (pthread_setschedparam(self, SCHED_FIFO, &p) != 0) {
        std::cout << "Warning: Cannot mark self as realtime, missing sudo?" << std::endl;
    } else {
        std::cout << "Info: Running with realtime priority " << p.sched_priority << " !" << std::endl;
    }
}
