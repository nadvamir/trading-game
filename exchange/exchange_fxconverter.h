#pragma once

#include <exchange_market.h>

namespace exchange {

class FXConverter {
    const Market& market;

public:
    FXConverter(const Market& market)
        : market(market)
    {
    }

    double convert(double amount, const std::string& from, const std::string& to) const
    {
        if (from == to) return amount;
        const auto quote = market.get_quote(from, to);
        const double rate = (quote.domestic == from) ? quote.mid : 1.0 / quote.mid;
        return amount * rate;
    }
};

} // namespace exchange

