#include "common.hpp"

struct lp_filter : public sink_source<1> {
    lp_filter();

    sink<1> &operator<<(const arr_t<1> &r) override;

    source<1> &operator>>(arr_t<1> &r) override;

private:
    static constexpr const size_t _order{ 30 };
    bool _empty;
    size_t _cursor;
    arr_t<_order + 1> _circular;
};
