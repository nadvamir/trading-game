#pragma once

#include <exchange_market.h>

namespace exchange {
class RandomTrader {
    Market& market;
    const long long min_trade;
    const long long max_trade;

public:
    RandomTrader(Market& market, long long min_trade, long long max_trade)
        : market(market), min_trade(min_trade), max_trade(max_trade)
    {
    }

    void trade()
    {
        const auto quote = market.get_a_random_quote();
        const long long trade = rand() % (max_trade - min_trade) + min_trade;
        if (rand() % 100 < 50) {
            market.buy(quote.domestic, quote.foreign, trade);
        }
        else {
            market.sell(quote.domestic, quote.foreign, trade);
        }
    }
};
} // namespace exchange

