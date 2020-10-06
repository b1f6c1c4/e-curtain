#include "common.hpp"
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/gpio.h>
#include <cstring>
#include <mutex>
#include <condition_variable>
#include "io/si7021.hpp"
#include "dsp/filter.hpp"
#include "net/udp_client.hpp"
#include "net/udp_server.hpp"
#include "sync.hpp"

using namespace std::chrono_literals;

int g_ofd;

// cur, win: percent to open/close
void write(double cur, double win, bool fan) {
    constexpr auto fast_clk{ 0.8ms };
    constexpr auto period{ 3ms };

    gpiohandle_data d{};
    d.values[1] = win == 0 ? 0 : 1; // ena
    d.values[3] = win < 0; // dir
    d.values[4] = fan ? 1 : 0; // f+
    d.values[5] = fan ? 0 : 1; // f-
    auto p_cur{ cur * 8s / (fast_clk * 4) };
    auto r_cur{ cur > 0 ? 3 : 1 };
    auto p_win{ win * 2500 };
    for (size_t i{ 0 }; i < p_cur || i < p_win; i++) {
        for (size_t j{ 0 }; j < 4; j++) {
            d.values[0] = i < p_cur && j < r_cur; // cur
            d.values[1] = i < p_win; // pul
            if (ioctl(g_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
                throw std::runtime_error("Cannot write gpio");
            std::this_thread::sleep_for(fast_clk / 2);
            d.values[1] = 0; // pul
            if (ioctl(g_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
                throw std::runtime_error("Cannot write gpio");
            std::this_thread::sleep_for(fast_clk / 2);
        }
    }
    d.values[0] = 0; // cur
    d.values[1] = 0; // ena
    d.values[2] = 0; // pul
    d.values[3] = 0; // dir
    d.values[4] = 0; // f+
    d.values[5] = 0; // f-
    if (ioctl(g_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
        throw std::runtime_error("Cannot write gpio");
}

int main(int argc, char *argv[]) {
    std::string host{ "controller-2" };
    if (argc == 1) {
        // use default host
    } else if (argc == 2) {
        host = argv[1];
    } else {
        std::cout << "Usage: ./H1-G1 [<host>]" << std::endl;
        std::cout << "Note: The default <host> is controller-2" << std::endl;
        return 1;
    }

    auto chipfd{ open("/dev/gpiochip0", 0) };
    if (chipfd < 0)
        throw std::runtime_error("Cannot open gpio dev");

    //             cur ena pul dir f+  f-
    //             8   12  11  13  7   10
    std::array po{ 14, 18, 17, 27, 4,  15 };
    gpiohandle_request req{};
    std::copy(po.begin(), po.end(), req.lineoffsets);
    req.flags = GPIOHANDLE_REQUEST_OUTPUT;
    req.lines = po.size();
    std::strcpy(req.consumer_label, "e-curtain-cxx");
    if (ioctl(chipfd, GPIO_GET_LINEHANDLE_IOCTL, &req) < 0)
        throw std::runtime_error("Cannot set gpio for write");
    g_ofd = req.fd;

    std::mutex mtx;
    std::condition_variable cv;
    udp_server<3> i_udp_server{ PORT };
    synchronizer<3> s_udp{ "s_udp", 0s, [&]() {
        arr_t<3> v;
        i_udp_server >> v;
        std::lock_guard l{ mtx };
        s_udp << v;
        cv.notify_one();
    }};
    synchronizer<3> s_gpio{ "s_gpio", 0s, [&]() {
        arr_t<3> prev, next;
        s_udp >> next;
        s_gpio >> prev;
        if (IS_INV(prev[0])) {
            prev[0] = -0.1;
            prev[1] = -0.1;
            prev[2] = 0;
            s_gpio << prev;
        }
        {
            std::unique_lock l{ mtx };
            while (prev == next || IS_INV(next[0])) {
                cv.wait(l);
                s_udp >> next;
            }
        }
        write(next[0] - prev[0], next[1] - prev[1], next[2]);
        std::this_thread::sleep_for(1s);
        s_gpio << next;
    }, true };

    si7021 i_sensor{ "/dev/i2c-1" };
    std::array<lp_filter, 2> lps;
    udp_client<3> i_udp_client{ host, PORT };
    synchronizer<0> s_sensor{ "s_sensor", 200ms, [&]() {
        arr_t<2> tr;
        i_sensor | lps;
        lps >> tr;
        i_udp_client << std::array{ 1.0, tr[0], tr[1] };
    }};
}
