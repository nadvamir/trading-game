#include <bandit/bandit.h>

#include <exchange_randomtrader.h>
#include <set>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 0.0001;

go_bandit([]{
describe("RandomTrader", []{
    const long long init_rate = 10000;
    Market::TradedPairs traded_pairs{
        Market::CP{new Currency{"GBP", "USD", 2ll, init_rate, 60'000'000'000}},
        Market::CP{new Currency{"EUR", "USD", 2ll, init_rate, 250'000'000'000}},
        Market::CP{new Currency{"EUR", "GBP", 2ll, init_rate, 20'000'000'000}},
        Market::CP{new Currency{"USD", "BTC", 2ll, init_rate, 1'000'000'000}},
    };
    Market market {traded_pairs, "USD"};

    it("moves the market with their trades", [&]{
        // GIVEN:
        const long long avg_trade = 5'000'000;
        RandomTrader trader(market, avg_trade);
        double min_rate = bp_to_double(init_rate);
        double max_rate = bp_to_double(init_rate);

        // WHEN:
        for (size_t i = 0; i < 10000; ++i) {
            trader.trade();
            double new_rate = market.get_quote("GBP", "USD").mid;
            min_rate = std::min(new_rate, min_rate);
            max_rate = std::max(new_rate, max_rate);
        }

        // THEN:
        AssertThat(double_to_bp(min_rate), IsLessThan(init_rate));
        AssertThat(double_to_bp(max_rate), IsGreaterThan(init_rate));
        AssertThat(market.get_quote("GBP", "USD").mid, !EqualsWithDelta(1.0, EPS));
        AssertThat(market.get_quote("EUR", "USD").mid, !EqualsWithDelta(1.0, EPS));
        AssertThat(market.get_quote("EUR", "GBP").mid, !EqualsWithDelta(1.0, EPS));
        AssertThat(market.get_quote("USD", "BTC").mid, !EqualsWithDelta(1.0, EPS));
    });
});
});

