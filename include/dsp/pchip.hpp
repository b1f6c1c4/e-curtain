#pragma once

#include "common.hpp"
#include <vector>

struct pchip {
    explicit pchip(const std::vector<std::pair<double, double>> &tbl);
    double operator[](double x) const;
private:
    std::vector<arr_t<5>> _pp;
};

struct cyc_pchip : private pchip {
    explicit cyc_pchip(double period, const std::vector<std::pair<double, double>> &tbl);
    double operator[](double x) const;
private:
    double _period;
};
