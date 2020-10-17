#include "dsp/sleep.hpp"

static double exteriorSlope(double d1, double d2, double h1, double h2)
{
    double s;
    double signd1;
    double signs;
    s = ((2.0 * h1 + h2) * d1 - h1 * d2) / (h1 + h2);
    signd1 = d1;
    if (d1 < 0.0) {
        signd1 = -1.0;
    } else if (d1 > 0.0) {
        signd1 = 1.0;
    } else {
        if (d1 == 0.0) {
            signd1 = 0.0;
        }
    }

    signs = s;
    if (s < 0.0) {
        signs = -1.0;
    } else if (s > 0.0) {
        signs = 1.0;
    } else {
        if (s == 0.0) {
            signs = 0.0;
        }
    }

    if (signs != signd1) {
        s = 0.0;
    } else {
        signs = d2;
        if (d2 < 0.0) {
            signs = -1.0;
        } else if (d2 > 0.0) {
            signs = 1.0;
        } else {
            if (d2 == 0.0) {
                signs = 0.0;
            }
        }

        if ((signd1 != signs) && (std::abs(s) > std::abs(3.0 * d1))) {
            s = 3.0 * d1;
        }
    }

    return s;
}

double g_sleep(double x)
{
    double y;
    int low_i;
    static const short iv[7] = { 0, 40, 70, 160, 430, 450, 460 };

    short h[6];
    double slopes[7];
    double del[6];
    int low_ip1;
    static const signed char iv1[7] = { 28, 28, 27, 26, 23, 26, 26 };

    int hs;
    int hs3;
    double dzzdx;
    double dzdxdx;
    double pp_coefs[24];
    static const short pp_breaks[7] = { 0, 40, 70, 160, 430, 450, 460 };

    for (low_i = 0; low_i < 6; low_i++) {
        short i;
        i = static_cast<short>(iv[low_i + 1] - iv[low_i]);
        h[low_i] = i;
        del[low_i] = static_cast<double>(iv1[low_i + 1] - iv1[low_i]) / static_cast<
                double>(i);
    }

    for (low_i = 0; low_i < 5; low_i++) {
        low_ip1 = h[low_i + 1];
        hs = h[low_i] + low_ip1;
        hs3 = 3 * hs;
        dzzdx = static_cast<double>(h[low_i] + hs) / static_cast<double>(hs3);
        dzdxdx = static_cast<double>(low_ip1 + hs) / static_cast<double>(hs3);
        slopes[low_i + 1] = 0.0;
        if (del[low_i] < 0.0) {
            double d;
            d = del[low_i + 1];
            if (d <= del[low_i]) {
                slopes[low_i + 1] = del[low_i] / (dzzdx * (del[low_i] / d) + dzdxdx);
            } else {
                if (d < 0.0) {
                    slopes[low_i + 1] = d / (dzzdx + dzdxdx * (d / del[low_i]));
                }
            }
        } else {
            if (del[low_i] > 0.0) {
                double d;
                d = del[low_i + 1];
                if (d >= del[low_i]) {
                    slopes[low_i + 1] = del[low_i] / (dzzdx * (del[low_i] / del[low_i + 1])
                                                      + dzdxdx);
                } else {
                    if (d > 0.0) {
                        slopes[low_i + 1] = del[low_i + 1] / (dzzdx + dzdxdx * (del[low_i +
                                                                                    1] / del[low_i]));
                    }
                }
            }
        }
    }

    slopes[0] = exteriorSlope(del[0], del[1], static_cast<double>(h[0]),
                              static_cast<double>(h[1]));
    slopes[6] = exteriorSlope(del[5], del[4], static_cast<double>(h[5]),
                              static_cast<double>(h[4]));
    for (low_i = 0; low_i < 6; low_i++) {
        dzzdx = (del[low_i] - slopes[low_i]) / static_cast<double>(h[low_i]);
        dzdxdx = (slopes[low_i + 1] - del[low_i]) / static_cast<double>(h[low_i]);
        pp_coefs[low_i] = (dzdxdx - dzzdx) / static_cast<double>(h[low_i]);
        pp_coefs[low_i + 6] = 2.0 * dzzdx - dzdxdx;
        pp_coefs[low_i + 12] = slopes[low_i];
        pp_coefs[low_i + 18] = iv1[low_i];
    }

    low_i = 0;
    low_ip1 = 2;
    hs = 7;
    while (hs > low_ip1) {
        hs3 = ((low_i + hs) + 1) >> 1;
        if (x >= pp_breaks[hs3 - 1]) {
            low_i = hs3 - 1;
            low_ip1 = hs3 + 1;
        } else {
            hs = hs3;
        }
    }

    dzzdx = x - static_cast<double>(pp_breaks[low_i]);
    y = dzzdx * (dzzdx * (dzzdx * pp_coefs[low_i] + pp_coefs[low_i + 6]) +
                 pp_coefs[low_i + 12]) + pp_coefs[low_i + 18];

    return y;
}
