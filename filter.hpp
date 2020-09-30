#include "common.hpp"

struct fir_filter : public sink_source<1> {
    fir_filter();
    sink<1> &operator<<(const arr_t<1> &r) override;

protected:
    static constexpr const size_t _order{ 30 };
    bool _empty;
    size_t _cursor;
    arr_t<_order+1> _circular;
};

struct lp_filter : public fir_filter {
    source<1> &operator>>(arr_t<1> &r) override;
};

struct df_filter : public fir_filter {
    source<1> &operator>>(arr_t<1> &r) override;
};
