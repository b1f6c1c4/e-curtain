#include "common.hpp"
#include <mutex>
#include "net/udp_client.hpp"
#include "net/udp_server.hpp"
#include "dsp/persistent.hpp"
#include "dsp/pwm.hpp"
#include "dsp/overlay.hpp"
#include "dsp/sleep.hpp"
#include "io/gpio.hpp"
#include "sync.hpp"

using namespace std::chrono_literals;

const arr_t<4> g_ABCD{ 1.0, 1.0, 1.0, 1.0 };
const arr_t<4> g_ABC{ 1.0, 1.0, 1.0, 0.0 };
const arr_t<4> g_AB{ 1.0, 1.0, 0.0, 0.0 };
const arr_t<4> g_CD{ 0.0, 0.0, 1.0, 1.0 };
const arr_t<4> g_AD{ 1.0, 0.0, 0.0, 1.0 };
const arr_t<4> g_A{ 1.0, 0.0, 0.0, 0.0 };
const arr_t<4> g_B{ 0.0, 1.0, 0.0, 0.0 };
const arr_t<4> g_C{ 0.0, 0.0, 1.0, 0.0 };
const arr_t<4> g_D{ 0.0, 0.0, 0.0, 1.0 };

auto time_of_day() {
    auto clk{ std::chrono::system_clock::now() };
    auto tt{ std::chrono::system_clock::to_time_t(clk) };
    auto tm{ *localtime(&tt) };
    return tm.tm_hour + (tm.tm_min + tm.tm_sec / 60.0) / 60.0;
}

struct state_machine_t : public sink<4>, public source<13> {
    state_machine_t() {
        arr_t<3> v{};
        _persistent >> v;
        if (!IS_INV(v[0])) {
            _state = static_cast<state_t>(v[0]);
            _offset = v[1];
            _slept = std::chrono::system_clock::time_point{ std::chrono::system_clock::duration{
                    *reinterpret_cast<const std::chrono::system_clock::rep *>(&v[2]) } };
            std::cout << "Info: recovered state " << _state;
            std::cout << " offset " << _offset;
            std::cout << " slept " << _slept.time_since_epoch().count();
            std::cout << std::endl;
        }
    }

    sink<4> &operator<<(const arr_t<4> &r) override {
        std::lock_guard l{ _mtx };
        if (r == g_AB)
            _offset += 0.25;
        else if (r == g_CD)
            _offset -= 0.25;
        else if (r == g_ABC)
            _offset = 0.0;
        switch (_state) {
            case S_NOBODY:
                if (r == g_ABCD)
                    _state = S_NORMAL;
                break;
            case S_NORMAL:
                if (r == g_ABCD)
                    _state = S_NOBODY;
                else if (r == g_B) {
                    if (time_of_day() >= 9.0 && time_of_day() < 20)
                        _state = S_SNAP;
                    else {
                        _state = S_SLEEP;
                        _slept = std::chrono::system_clock::now();
                    }
                }
                break;
            case S_SNAP:
                if (r == g_A)
                    _state = S_NORMAL;
                else if (!(time_of_day() >= 9.0 && time_of_day() < 20))
                    _state = S_SNAP;
                break;
            case S_SLEEP:
            case S_RSNAP:
                if (r == g_A)
                    _state = S_NORMAL;
                else if (r == g_B)
                    _state = S_SLEEP;
                else if (r == g_C)
                    _state = S_RSNAP;
                else if (r == g_D)
                    _slept += 30min;
                else if (r == g_AD)
                    _slept -= 30min;
                break;
        }
        arr_t<3> v{};
        v[0] = static_cast<double>(_state);
        v[1] = _offset;
        auto sl{ _slept.time_since_epoch().count() };
        v[2] = *reinterpret_cast<const double *>(&sl);
        _persistent << v;
        return *this;
    }

    source<13> &operator>>(arr_t<13> &r) override {
        std::lock_guard l{ _mtx };
        auto td{ time_of_day() };
        auto ts{ std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::system_clock::now() - _slept).count() / 60.0 };
        r[0] = 3;
        switch (_state) {
            case S_NOBODY:
                r[1] = 20.0, r[2] = 20.0; // tp[12]
                r[3] = 0.0, r[4] = 0.0; // f012b[lu]
                r[5] = 0.5, r[6] = 0.5; // curb[lu]
                r[7] = 0.5, r[8] = 0.0, r[9] = 0.0; // w[012]
                break;
            case S_NORMAL:
                if (td >= 9.0 && td < 20.0) {
                    r[1] = 26.0, r[2] = 26.0; // tp[12]
                    r[3] = 0.0, r[4] = 1.0; // f012b[lu]
                    r[5] = 0.0, r[6] = 1.0; // curb[lu]
                    r[7] = 1.0, r[8] = 0.0, r[9] = 3.0; // w[012]
                } else if (td >= 20.0 && td < 23.5) {
                    r[1] = 26.0, r[2] = 25.5; // tp[12]
                    r[3] = 0.0, r[4] = 1.0; // f012b[lu]
                    r[5] = 0.0, r[6] = 1.0; // curb[lu]
                    r[7] = 1.0, r[8] = 0.3, r[9] = 2.7; // w[012]
                } else if (td >= 23.5 && td < 23.55) {
                    r[1] = 26.0, r[2] = 25.5; // tp[12]
                    r[3] = 1.0, r[4] = 1.0; // f012b[lu]
                    r[5] = 0.0, r[6] = 0.0; // curb[lu]
                    r[7] = 1.0, r[8] = 0.5, r[9] = 2.5; // w[012]
                } else {
                    r[1] = 26.0, r[2] = 26.0; // tp[12]
                    r[3] = 0.0, r[4] = 0.2; // f012b[lu]
                    r[5] = 0.0, r[6] = 0.0; // curb[lu]
                    r[7] = 1.0, r[8] = 0.5, r[9] = 2.5; // w[012]
                }
                break;
            case S_SNAP:
                r[1] = 26.0, r[2] = 26.0; // tp[12]
                r[3] = 0.0, r[4] = 0.2; // f012b[lu]
                r[5] = 0.5, r[6] = 0.5; // curb[lu]
                r[7] = 1.0, r[8] = 1.0, r[9] = 2.0; // w[012]
                break;
            case S_SLEEP:
            case S_RSNAP:
                r[1] = g_sleep(ts);
                if (ts < 480)
                    r[3] = 0.0, r[4] = 0.0; // f012b[lu]
                else
                    r[3] = 0.1, r[4] = 0.1; // f012b[lu]
                if (ts < 480)
                    r[5] = r[6] = 0; // curb[lu]
                else
                    r[5] = r[6] = 1; // curb[lu]
                if (_state == S_SLEEP) {
                    r[2] = r[1] - 0.5; // tp2
                    r[7] = 1.0, r[8] = 3.0, r[9] = 0.0; // w[012]
                } else { // if (_state == S_RSNAP)
                    r[2] = std::max(26.0, r[1] - 1.0); // tp2
                    r[7] = 1.0, r[8] = 2.0, r[9] = 1.0; // w[012]
                }
                break;
        }
        r[1] += _offset, r[2] += _offset;
        r[10] = static_cast<double>(_state);
        r[11] = ts;
        r[12] = _offset;
        return *this;
    }

    void offset(double d) {
        std::lock_guard l{ _mtx };
        if (d == +1)
            _offset += 0.25;
        else if (d == -1)
            _offset -= 0.25;
        else
            _offset = 0.0;
    }

private:
    std::mutex _mtx;
    enum state_t {
        S_NOBODY = 0,
        S_NORMAL = 1,
        S_SNAP = 2,
        S_SLEEP = 3,
        S_RSNAP = 4,
    } _state{ S_NORMAL };
    double _offset{ 0.0 };
    std::chrono::system_clock::time_point _slept;
    persistent<3> _persistent{ "F-G0.bin" };
};

int main(int argc, char *argv[]) {
    std::string host;
    if (argc == 2) {
        host = argv[1];
    } else {
        std::cout << "Usage: ./F-G0 <host>" << std::endl;
        return 1;
    }

    std::mutex mtx;
    gpio<4, 4> i_gpio{
            "/dev/gpiochip0",
            // Inputs:
            // A   B   C   D
            // 32  36  38  40
            { 12, 16, 20, 21 },
            // Outputs:
            // acp acm r1  r2
            // 12  10  8   16
            { 18, 15, 14, 23 },
            // Output Slew Limits:
            { false, false, false, true },
    };
    overlay<4> i_overlay{};

    auto write{ [&](double acp, double acm, double reg1, double reg2) {
        std::cout << "Write: " << acp << " " << acm << " " << reg1 << " " << reg2 << std::endl;
        auto m{ 60.0 };
        auto p{ 180.0 * acp };
        switch (static_cast<int>(acm)) {
            case 2:
                p = 180.0 - p;
                m = 0.0;
                break;
            case 1:
                p = 180.0 - p;
                m = 30.0;
                break;
            case 0:
                m = 60.0;
                break;
            case -1:
                m = 150.0;
                break;
            case -2:
                m = 180.0;
                break;
            default:
                std::cout << "Warning: Invalid acm" << std::endl;
                break;
        }
        i_overlay << arr_t<4>{ p, m, 180.0 - 100.0 * reg1, 77.0 * reg2 };
        i_overlay | i_gpio;
    } };

    state_machine_t sm;

    udp_client<13> i_udp_client{ host, PORT };
    synchronizer<0> s_sp{ "s_sp", 0s, [&]() {
        i_gpio | sm;
        sm | i_udp_client;
    } };
    arr_t<13> old_sp{};
    synchronizer<0> s_spt{ "s_spt", 10s, [&]() {
        sm << arr_t<4>{};
        arr_t<13> sp{};
        sm >> sp;
        if (sp != old_sp)
            std::cout << "Set point: " << sp << std::endl;
        old_sp = sp;
        i_udp_client << sp;
    } };

    udp_server<4> i_udp_server{ PORT };
    pwm i_pwm{ 60 }; // 60 x 2s
    synchronizer<1> s_pwm{ "s_pwm", 2s, };
    synchronizer<3> s_recv{ "s_recv", 0s, [&]() {
        arr_t<4> v;
        i_udp_server >> v;
        if (IS_INV(v[0])) {
            sm.offset(v[1]);
        } else {
            i_pwm << arr_t<1>{ v[1] }; // acp
            s_recv << arr_t<3>{ v[0], v[2], v[3] }; // acm reg1 reg2
            arr_t<1> pv;
            s_pwm >> pv;
            std::lock_guard l{ mtx };
            write(pv[0], v[0], v[2], v[3]);
        }
    } };
    s_recv << arr_t<3>{ 0.0, 0.0, 0.0 };
    s_pwm.set_callback([&]() {
        arr_t<1> v;
        i_pwm >> v;
        if (!IS_INV(v[0])) {
            s_pwm << v;
            arr_t<3> sv;
            s_recv >> sv;
            std::lock_guard l{ mtx };
            write(v[0], sv[0], sv[1], sv[2]);
        }
    });
}
