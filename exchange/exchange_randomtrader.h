#pragma once

#include <exchange_market.h>
#include <random>

namespace exchange {
class RandomTrader {
    Market& market;
    const long long avg_trade;

    std::random_device rd;
    std::mt19937 gen;
    std::normal_distribution<> d;

public:
    RandomTrader(Market& market, long long avg_trade)
        : market(market)
        , avg_trade(avg_trade)
        , rd{}
        , gen(rd())
        , d(avg_trade, avg_trade * 0.25)
    {
    }

    void trade()
    {
        const auto quote = market.get_a_random_quote();
        const long long trade = std::max(1.0, d(gen));
        if (rand() % 100 < 50) {
            market.buy(quote.domestic, quote.foreign, trade);
        }
        else {
            market.sell(quote.domestic, quote.foreign, trade);
        }
    }
};
} // namespace exchange

