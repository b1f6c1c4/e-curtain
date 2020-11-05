#include "common.hpp"
#include "params.hpp"
#include <mutex>
#include "net/udp_client.hpp"
#include "net/udp_server.hpp"
#include "dsp/persistent.hpp"
#include "dsp/pwm.hpp"
#include "dsp/overlay.hpp"
#include "dsp/pchip.hpp"
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

struct state_machine_t : public sink<4>, public source<sp_size> {
    state_machine_t() {
        arr_t<4> v{};
        _persistent >> v;
        if (!IS_INV(v[0])) {
            _state = static_cast<state_t>(v[0]);
            _offset = v[1];
            _offset2 = v[2];
            _slept = std::chrono::system_clock::time_point{ std::chrono::system_clock::duration{
                    *reinterpret_cast<const std::chrono::system_clock::rep *>(&v[3]) } };
            std::cout << "Info: recovered state " << _state;
            std::cout << " offset " << _offset;
            std::cout << " offset2 " << _offset2;
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
        arr_t<4> v{};
        v[0] = static_cast<double>(_state);
        v[1] = _offset;
        v[2] = _offset2;
        auto sl{ _slept.time_since_epoch().count() };
        v[3] = *reinterpret_cast<const double *>(&sl);
        _persistent << v;
        return *this;
    }

    source<sp_size> &operator>>(arr_t<sp_size> &r) override {
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
                r[1] = _normal_tp1[td], r[2] = _normal_tp2[td]; // tp[12]
                for (size_t i{ 1 }; i < prediction_horizon; i++) {
                    auto v1{ _normal_tp1[td + static_cast<double>(i) * 10 / 60 / 60] + _offset };
                    auto v2{ _normal_tp2[td + static_cast<double>(i) * 10 / 60 / 60] + _offset };
                    auto ptr{ reinterpret_cast<float *>(&r[13 + i]) };
                    ptr[0] = static_cast<float>(v1);
                    ptr[1] = static_cast<float>(v2);
                }
                if (td >= 3.0 && td < 9.0) {
                    r[3] = 0.0, r[4] = 0.2; // f012b[lu]
                    r[5] = 0.0, r[6] = 1.0; // curb[lu]
                    r[9] = 2.0 + (td - 3.0) / (9.0 - 3.0); // w2
                } else if (td >= 9.0 && td < 20.0) {
                    r[3] = 0.0, r[4] = 1.0; // f012b[lu]
                    r[5] = 0.0, r[6] = 1.0; // curb[lu]
                    r[9] = 3.0; // w2
                } else if (td >= 20.0 && td < 23.5) {
                    r[3] = 0.0, r[4] = 1.0; // f012b[lu]
                    r[5] = 0.0, r[6] = 1.0; // curb[lu]
                    r[9] = 3.0 - (td - 20.0) / (23.5 - 20.0); // w2
                } else {
                    r[3] = 0.0, r[4] = 0.2; // f012b[lu]
                    r[5] = 0.0, r[6] = 0.0; // curb[lu]
                    r[9] = 2.0; // w2
                }
                r[7] = 1.0, r[8] = 3.0 - r[9]; // w[01]
                break;
            case S_SNAP:
                r[1] = 26.0, r[2] = _normal_tp2[td]; // tp[12]
                r[3] = 0.0, r[4] = 0.0; // f012b[lu]
                r[5] = 0.5, r[6] = 0.5; // curb[lu]
                r[7] = 1.0, r[8] = 1.0, r[9] = 2.0; // w[012]
                break;
            case S_SLEEP:
            case S_RSNAP:
                r[1] = _sleep[ts], r[2] = _normal_tp2[td]; // tp[12]
                if (ts < 480) {
                    r[3] = 0.0, r[4] = 0.0; // f012b[lu]
                    r[5] = r[6] = 0; // curb[lu]
                    r[8] = 3.0; // w1
                } else if (ts < 481) {
                    r[3] = 0.19, r[4] = 0.19; // f012b[lu]
                    r[5] = r[6] = 0; // curb[lu]
                    r[8] = 3.0; // w1
                } else {
                    r[3] = 0.0, r[4] = 0.0; // f012b[lu]
                    r[5] = r[6] = 1; // curb[lu]
                    r[8] = std::max(1.5, 3.0 - 1.5 * (ts - 481) / 15); // w1
                }
                if (_state == S_RSNAP) {
                    r[8] = 2.0; // w1
                }
                r[7] = 1.0, r[9] = 3.0 - r[8]; // w[02]
                break;
        }
        r[1] += _offset, r[2] += _offset + _offset2;
        switch (_state) {
            case S_NORMAL:
                for (size_t i{ 1 }; i < prediction_horizon; i++) {
                    auto v1{ _normal_tp1[td + static_cast<double>(i) * 10 / 60 / 60] + _offset };
                    auto v2{ _normal_tp2[td + static_cast<double>(i) * 10 / 60 / 60] + _offset };
                    auto ptr{ reinterpret_cast<float *>(&r[13 + i]) };
                    ptr[0] = static_cast<float>(v1 + _offset);
                    ptr[1] = static_cast<float>(v2 + _offset + _offset2);
                }
                break;
            case S_SNAP:
                for (size_t i{ 1 }; i < prediction_horizon; i++) {
                    auto v1{ _normal_tp1[td + static_cast<double>(i) * 10 / 60 / 60] + _offset };
                    auto v2{ _normal_tp2[td + static_cast<double>(i) * 10 / 60 / 60] + _offset };
                    auto ptr{ reinterpret_cast<float *>(&r[13 + i]) };
                    ptr[0] = static_cast<float>(v1 + _offset);
                    ptr[1] = static_cast<float>(v2 + _offset + _offset2);
                }
                break;
            case S_SLEEP:
            case S_RSNAP:
                for (size_t i{ 1 }; i < prediction_horizon; i++) {
                    auto v1{ _sleep[ts + static_cast<double>(i) * 10 / 60] + _offset };
                    auto v2{ _normal_tp2[td + static_cast<double>(i) * 10 / 60 / 60] + _offset };
                    auto ptr{ reinterpret_cast<float *>(&r[13 + i]) };
                    ptr[0] = static_cast<float>(v1 + _offset);
                    ptr[1] = static_cast<float>(v2 + _offset + _offset2);
                }
                break;
            default:
                for (size_t i{ 1 }; i < prediction_horizon; i++) {
                    auto ptr{ reinterpret_cast<float *>(&r[13 + i]) };
                    ptr[0] = r[1];
                    ptr[1] = r[2];
                }
                break;
        }
        r[10] = static_cast<double>(_state);
        r[11] = ts;
        r[12] = _offset;
        r[13] = _offset2;
        return *this;
    }

    void offset2(double d) {
        std::lock_guard l{ _mtx };
        if (d == +1)
            _offset2 += 0.25;
        else if (d == -1)
            _offset2 -= 0.25;
        else
            _offset2 = 0.0;
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
    double _offset{ 0.0 }, _offset2{ 0.0 };
    std::chrono::system_clock::time_point _slept;
    persistent<4> _persistent{ "F-G0.bin" };

    cyc_pchip _normal_tp1{ 24, {
            { 0.0, 26.0 },
            { 3.0, 26.0 },
            { 11.0, 26.0 },
            { 14.0, 26.25 },
            { 20.0, 25.75 },
            { 23.5, 25.75 } } };
    cyc_pchip _normal_tp2{ 24, {
            { 0.0, 25.5 },
            { 3.0, 26.0 },
            { 11.0, 26.0 },
            { 14.0, 26.25 },
            { 20.0, 25.75 },
            { 23.5, 25.75 } } };
    pchip _sleep{ {
            { 0, 28 },
            { 40, 28 },
            { 70, 27 },
            { 160, 26 },
            { 430, 23.75 },
            { 450, 26 },
            { 460, 26 } } };
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
        auto m{ 60.0 };
        auto p{ 180.0 * acp };
        switch (static_cast<int>(acm)) {
            case 2:
                p = 180.0 - p;
                m = 0.0;
                break;
            case 1:
                p = 180.0 - p;
                m = 25.0;
                break;
            case 0:
                m = 60.0;
                break;
            case -1:
                m = 155.0;
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

    udp_client<sp_size> i_udp_client{ host, PORT };
    synchronizer<0> s_sp{ "s_sp", 0s, [&]() {
        i_gpio | sm;
        sm | i_udp_client;
    } };
    synchronizer<0> s_spt{ "s_spt", 10s, [&]() {
        sm << arr_t<4>{};
        arr_t<sp_size> sp{};
        sm >> sp;
        i_udp_client << sp;
    } };

    udp_server<4> i_udp_server{ PORT };
    pwm i_pwm{ 60 }; // 60 x 2s
    synchronizer<1> s_pwm{ "s_pwm", 2s, };
    synchronizer<3> s_recv{ "s_recv", 0s, [&]() {
        arr_t<4> v;
        i_udp_server >> v;
        if (IS_INV(v[0])) {
            sm.offset2(v[1]);
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
