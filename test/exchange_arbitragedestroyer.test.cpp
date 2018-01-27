#include <bandit/bandit.h>

#include <exchange_market.h>

#include <set>
#include <cmath>
#include <algorithm>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 0.00001;

struct ArbitrageDestroyer {
    static std::array<Quote, 3> normalise(std::array<Quote, 3> quotes)
    {
        std::random_shuffle(begin(quotes), end(quotes));
        normalise(quotes[0], quotes[1], quotes[2]);
        return quotes;
    }

private:
    static void normalise(Quote& quote_to_modify, const Quote& q1, const Quote& q2)
    {
        const bool invert_r1 = (quote_to_modify.domestic != q1.domestic && 
                                quote_to_modify.foreign != q1.foreign);
        const bool invert_r2 = (quote_to_modify.domestic != q2.domestic && 
                                quote_to_modify.foreign != q2.foreign);

        const double rate1 = invert_r1 ? 1.0 / q1.mid : q1.mid;
        const double rate2 = invert_r2 ? 1.0 / q2.mid : q2.mid;

        quote_to_modify.mid = rate1 * rate2;
    }
};

go_bandit([]{
describe("ArbitrageDestroyer", []{
    Market::TradedPairs traded_pairs{
        Market::CP{new Currency{"GBP", "USD", 2ll, 5000, 60'000'000'000}},
        Market::CP{new Currency{"EUR", "USD", 2ll, 6000, 250'000'000'000}},
        Market::CP{new Currency{"EUR", "GBP", 2ll, 11000, 20'000'000'000}},
        Market::CP{new Currency{"USD", "BTC", 2ll, 5000, 1'000'000'000}},
    };
    Market market {traded_pairs, "USD"};

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
        auto quotes = market.get_a_random_triangle();
        // WHEN:
        auto normalised_quotes = ArbitrageDestroyer::normalise(quotes);
        // THEN:
        AssertThat(has_arbitrage(quotes), Equals(true));
        AssertThat(has_arbitrage(normalised_quotes), Equals(false));
    });

    it("chooses a random currency to modify", [&]{
        // GIVEN:
        std::set<double> rates;
        // WHEN:
        for (size_t i = 0; i < 1000; ++i) {
            auto quotes = market.get_a_random_triangle();
            quotes = ArbitrageDestroyer::normalise(quotes);
            AssertThat(has_arbitrage(quotes), Equals(false));
            rates.insert(quotes[0].mid);
            rates.insert(quotes[1].mid);
            rates.insert(quotes[2].mid);
        }
        // THEN:
        AssertThat(rates.size(), Equals(6));
    });
});
});

