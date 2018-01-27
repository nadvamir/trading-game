#pragma once

#include <exchange_market.h>
#include <algorithm>

namespace exchange {

struct ArbitrageDestroyer {
    static void normalise(Market& market)
    {
        auto quotes = market.get_a_random_triangle();
        std::random_shuffle(begin(quotes), end(quotes));
        const double new_rate = normalise(quotes[0], quotes[1], quotes[2]);
        market.set_rate(quotes[0].domestic, quotes[0].foreign, new_rate);
    }

private:
    static double normalise(const Quote& quote_to_modify, const Quote& q1, const Quote& q2)
    {
        const bool invert_r1 = (quote_to_modify.domestic != q1.domestic && 
                                quote_to_modify.foreign != q1.foreign);
        const bool invert_r2 = (quote_to_modify.domestic != q2.domestic && 
                                quote_to_modify.foreign != q2.foreign);

        const double rate1 = invert_r1 ? 1.0 / q1.mid : q1.mid;
        const double rate2 = invert_r2 ? 1.0 / q2.mid : q2.mid;

        return rate1 * rate2;
    }
};

} // namespace exchange
