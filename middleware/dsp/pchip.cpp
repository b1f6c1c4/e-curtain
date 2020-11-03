#include "dsp/pchip.hpp"

pchip::pchip(const std::vector<std::pair<double, double>> &tbl) {
    auto sz = tbl.size() - 1;

    std::vector<double> h, slopes, del;
    for (size_t i{ 0 }; i < sz; i++) {
        auto tmp = tbl[i + 1].first - tbl[i].first;
        h.push_back(tmp);
        del.push_back((tbl[i + 1].second - tbl[i].second) / tmp);
    }
    slopes.push_back(0);
    for (size_t i{ 1 }; i < sz; i++) {
        auto hs = h[i - 1] + h[i];
        auto hs3 = 3 * hs;
        auto dzzdx = (h[i - 1] + hs) / (hs3);
        auto dzdxdx = (h[i] + hs) / (hs3);
        auto sl = 0.0;
        if (del[i - 1] < 0.0) {
            if (del[i] <= del[i - 1])
                sl = del[i - 1] / (dzzdx * (del[i - 1] / del[i]) + dzdxdx);
            else if (del[i] < 0.0)
                sl = del[i] / (dzzdx + dzdxdx * (del[i] / del[i - 1]));
        } else if (del[i - 1] > 0.0) {
            if (del[i] >= del[i - 1])
                sl = del[i - 1] / (dzzdx * (del[i - 1] / del[i]) + dzdxdx);
            else if (del[i] > 0.0)
                sl = del[i] / (dzzdx + dzdxdx * (del[i] / del[i - 1]));
        }
        slopes.push_back(sl);
    }
    slopes.push_back(0);
    _pp.reserve(4 * sz);
    for (size_t i{ 0 }; i < sz; i++) {
        auto dzzdx = (del[i] - slopes[i]) / h[i];
        auto dzdxdx = (slopes[i + 1] - del[i]) / h[i];
        _pp.emplace_back(arr_t<5>{
            tbl[i].first,
            (dzdxdx - dzzdx) / h[i],
            2.0 * dzzdx - dzdxdx,
            slopes[i],
            tbl[i].second });
    }
}

double pchip::operator[](double x) const {
    size_t id;
    for (id = 0; id < _pp.size() - 1; id++)
        if (x < _pp[id + 1][0])
            break;
    auto p = _pp[id];
    auto d = x - p[0];
    return d * (d * (d * p[1] + p[2]) + p[3]) + p[4];
}

cyc_pchip::cyc_pchip(double period, const std::vector<std::pair<double, double>> &tbl) : pchip{ [&](){
    auto tb{ tbl };
    tb.insert(tb.begin(), std::make_pair(tbl.back().first - period, tbl.back().second));
    tb.push_back(std::make_pair(tbl[0].first + period, tbl[0].second));
    tb.push_back(std::make_pair(tbl[1].first + period, tbl[1].second));
    return tb;
}() }, _period{ period } { }

double cyc_pchip::operator[](double x) const {
    return pchip::operator[](x - _period * std::floor(x / _period));
}
