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
    std::cout << "Write: " << cur << " " << win << " " << fan << std::endl;
    constexpr auto fast_clk{ 0.8ms };
    constexpr auto period{ 3ms };
    constexpr auto min_cyc{ 300ms / fast_clk / 4 };

    gpiohandle_data d{};
    d.values[1] = win == 0 ? 0 : 1; // ena
    d.values[3] = win < 0; // dir
    d.values[4] = fan ? 1 : 0; // f+
    d.values[5] = fan ? 0 : 1; // f-
    auto p_cur{ std::abs(cur) * 6s / (fast_clk * 4) };
    auto r_cur{ cur > 0 ? 1 : 3 };
    auto p_win{ std::abs(win) * 2150 };
    auto len{ min_cyc };
    if (p_cur > len) len = p_cur;
    if (p_win > len) len = p_win;
    for (size_t i{ 0 }; i < len; i++) {
        for (size_t j{ 0 }; j < 4; j++) {
            d.values[0] = i < p_cur && j < r_cur; // cur
            d.values[2] = i < p_win; // pul
            if (ioctl(g_ofd, GPIOHANDLE_SET_LINE_VALUES_IOCTL, &d) < 0)
                throw std::runtime_error("Cannot write gpio");
            std::this_thread::sleep_for(fast_clk / 2);
            d.values[2] = 0; // pul
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
    std::string host;
    if (argc == 2) {
        host = argv[1];
    } else {
        std::cout << "Usage: ./H1-G1 <host>" << std::endl;
        return 1;
    }

    auto chipfd{ open("/dev/gpiochip0", 0) };
    if (chipfd < 0)
        throw std::runtime_error("Cannot open gpio dev");

    //             cur ena pul dir f+  f-
    //             8   12  11  13  7   10
    std::array po{ 14, 18, 17, 27, 4, 15 };
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

    synchronizer<3> s_udp{ "s_udp", 0s, };
    s_udp << arr_t<3>{ 0.0, 0.0, 0.0 };

    synchronizer<3> s_gpio{ "s_gpio", 0s, true };
    s_gpio << arr_t<3>{ 0.0, 0.0, 0.0 };

    s_udp.set_callback([&]() {
        arr_t<3> v;
        i_udp_server >> v;
        std::lock_guard l{ mtx };
        if (IS_INV(v[2])) { // calibration
            arr_t<3> prev;
            s_gpio >> prev;
            prev[0] = v[0];
            prev[1] = v[1];
            std::cout << "Calibration: " << prev << std::endl;
            s_gpio << prev;
        } else {
            s_udp << v;
        }
        cv.notify_one();
    });

    s_gpio.set_callback([&]() {
        arr_t<3> prev, next;
        s_udp >> next;
        s_gpio >> prev;

        {
            std::unique_lock l{ mtx };
            while (prev == next || IS_INV(next[0])) {
                cv.wait(l);
                s_udp >> next;
                s_gpio >> prev;
            }
        }

        if (IS_INV(next[0]))
            next[0] = 0.5;
        else if (next[0] > 1)
            next[0] = 1;

        if (IS_INV(next[1]))
            next[1] = 0.5;
        else if (next[1] > 1)
            next[1] = 1;

        write(next[0] - std::max(prev[0], 0.0), std::max(next[1], 0.0) - std::max(prev[1], 0.0), next[2]);
        std::this_thread::sleep_for(1s);
        {
            std::unique_lock l{ mtx };
            s_gpio << next;
        }
    });

    si7021 i_sensor{ "/dev/i2c-1" };
    std::array<lp_filter, 2> lps;
    udp_client<3> i_udp_client{ host, PORT };
    synchronizer<0> s_sensor{ "s_sensor", 200ms, [&]() {
        arr_t<2> tr;
        i_sensor | lps;
        lps >> tr;
        i_udp_client << std::array{ 1.0, tr[0], tr[1] };
    } };
}