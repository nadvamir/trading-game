#pragma once

#include <exchange_fxconversions.h>

#include <atomic>
#include <string>
#include <sstream>
#include <iomanip>
#include <cassert>

namespace exchange {

struct Quote {
    std::string ccy_pair;
    std::string domestic;
    std::string foreign;
    long long spread;
    long long volume;
    double bid;
    double mid;
    double ask;

    Quote invert() const
    {
        return Quote {
            foreign + domestic, foreign, domestic, spread, volume,
            1/bid, 1/mid, 1/ask
        };
    }

    std::string to_string() const
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

    friend std::ostream& operator<<(std::ostream& os, const Quote& q)
    {
        return os
            << q.ccy_pair << " "
            << std::fixed << std::setprecision(4)
            << q.bid << " " << q.ask;
    }
};

class Currency {
    std::string domestic;
    std::string foreign;
    long long spread;
    mutable std::atomic_llong rate_bp;
    long long init_rate;
    long long volume;

public:
    Currency(
            const std::string& domestic,
            const std::string& foreign,
            long long spread,
            long long rate,
            long long volume = 10'000'000'000)
        : domestic(domestic)
        , foreign(foreign)
        , spread(spread)
        , rate_bp(rate)
        , init_rate(rate)
        , volume(volume)
    {
    }

    Quote quote() const
    {
        const double half_spread = bp_to_double(spread) / 2.0;
        const double mid = bp_to_double(rate_bp.load());
        return Quote {
            domestic + foreign, domestic, foreign, spread, volume,
            mid - half_spread, mid, mid + half_spread
        };
    }

    void buy(double amount) const
    {
        assert(amount > 0);
        rate_bp.fetch_add(calculate_move(amount, 1));
    }

    void sell(double amount) const
    {
        assert(amount > 0);
        rate_bp.fetch_add(calculate_move(amount, -1));
    }

    void set_rate(double rate) const
    {
        rate_bp.store(double_to_bp(rate));
    }

    bool is(const std::string& currency) const
    {
        return foreign == currency || domestic == currency;
    }

    bool is(const std::string& ccy1, const std::string& ccy2) const
    {
        return (foreign == ccy1 && domestic == ccy2)
            || (foreign == ccy2 && domestic == ccy1);
    }

private:
    long long calculate_move(double amount, long long direction) const
    {
        double delta = amount / double(volume) * 2000;
        if (amount >= 1'000'000) delta += 1;
        const bool rebounding = rate_bp * 1.2 < init_rate && direction > 0;
        const bool dampening = rate_bp * 0.8 > init_rate && direction < 0;
        if (rebounding || dampening) delta *= 2;
        return delta * direction;
    }
};

} // namespace exchange

