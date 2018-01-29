#include <bandit/bandit.h>

#include <exchange_arbitragedestroyer.h>

#include <set>
#include <cmath>
#include <algorithm>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 1e-4;

go_bandit([]{
describe("ArbitrageDestroyer", []{
    auto get_a_market = [] {
        Market::TradedPairs traded_pairs {
            Market::CP{new Currency{"GBP", "USD", 2ll, 5000, 60'000'000'000}},
            Market::CP{new Currency{"EUR", "USD", 2ll, 6000, 250'000'000'000}},
            Market::CP{new Currency{"EUR", "GBP", 2ll, 11000, 20'000'000'000}},
            Market::CP{new Currency{"USD", "BTC", 2ll, 5000, 1'000'000'000}},
        };
        return Market {traded_pairs, "USD"};
    };

    auto get = [](auto quotes, auto ccy_pair) {
        return *std::find_if(begin(quotes), end(quotes), [&](auto q) {
                return q.ccy_pair == ccy_pair;
        });
    };

    auto has_arbitrage = [&](auto quotes) {
        const double rate = get(quotes, "EURUSD").mid / get(quotes, "GBPUSD").mid;
        return fabs(get(quotes, "EURGBP").mid - rate) >= EPS;
    };

    it("removes arbitrage", [&]{
        // GIVEN:
        auto market = get_a_market();
        auto quotes = market.get_a_random_triangle();
        AssertThat(has_arbitrage(quotes), Equals(true));

        // WHEN:
        ArbitrageDestroyer::normalise(market);

        // THEN:
        auto normalised_quotes = market.get_a_random_triangle();
        AssertThat(has_arbitrage(normalised_quotes), Equals(false));
    });

    it("chooses a random currency to modify", [&]{
        // GIVEN:
        std::set<double> rates;

        // WHEN:
        for (size_t i = 0; i < 1000; ++i) {
            auto market = get_a_market();
            ArbitrageDestroyer::normalise(market);
            auto quotes = market.get_a_random_triangle();
            AssertThat(has_arbitrage(quotes), Equals(false));
            rates.insert(quotes[0].mid);
            rates.insert(quotes[1].mid);
            rates.insert(quotes[2].mid);
        }

        // THEN:
        AssertThat(rates.size(), Equals(6ul));
    });
});
});

