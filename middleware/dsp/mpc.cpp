#include "dsp/mpc.hpp"

mpc::mpc() {
    _controller.initialize();
}

sink<15> &mpc::operator<<(const arr_t<15> &r) {
    // TODO: write inputs
    // Note: the sampling time should be 10ms
    libdumbacModelClass::ExtU u{};
    _controller.setExternalInputs(&u);
    _controller.step();
    return *this;
}

source<7> &mpc::operator>>(arr_t<7> &r) {
    auto g{ _controller.getExternalOutputs().g };
    std::copy(g, g + 7, r.begin());
    return *this;
}
