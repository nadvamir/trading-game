#pragma once

#include <atomic>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace exchange {

struct Quote {
    std::string ccyPair;
    double bid;
    double mid;
    double ask;

    friend std::ostream& operator<<(std::ostream& os, const Quote& q)
    {
        return os
            << q.ccyPair << " "
            << std::fixed << std::setprecision(4)
            << q.bid << " " << q.ask;
    }
};

class Currency {
    std::string domestic;
    std::string foreign;
    bool inverted;
    long long spread;
    mutable std::atomic_llong rate_bp;
    long long volume;

public:
    Currency(
            const std::string& domestic,
            const std::string& foreign,
            bool inverted,
            long long spread,
            long long rate,
            long long volume = 10'000'000'000)
        : domestic(domestic)
        , foreign(foreign)
        , inverted(inverted)
        , spread(spread)
        , rate_bp(rate)
        , volume(volume)
    {
    }

    Quote quote() const
    {
        const double half_spread = double(spread) / 10000.0 / 2.0;
        if (inverted) {
            const double mid = 1.0 / (double(rate_bp.load()) / 10000.0);
            return Quote{foreign + domestic, mid - half_spread, mid, mid + half_spread};
        }
        else {
            const double mid = double(rate_bp.load()) / 10000.0;
            return Quote{domestic + foreign, mid - half_spread, mid, mid + half_spread};
        }
    }

    void buy(long long amount) const
    {
        rate_bp.fetch_add(calculate_move(amount));
    }

    void sell(long long amount) const
    {
        rate_bp.fetch_add(-calculate_move(amount));
    }

    void set_rate(double rate) const
    {
        rate_bp.store(round(rate * 10000));
    }

    bool is(const std::string& currency) const
    {
        return foreign == currency || domestic == currency;
    }

    bool is(const std::string& ccy1, const std::string& ccy2) const
    {
        return foreign == ccy1 && domestic == ccy2
            || foreign == ccy2 && domestic == ccy1;
    }

private:
    long long calculate_move(long long amount) const
    {
        const double increase = 1.0 + 10 * double(amount) / double(volume);
        return 10000 * increase * increase; 
    }
};

} // namespace exchange

