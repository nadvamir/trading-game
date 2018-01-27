#include <bandit/bandit.h>

#include <exchange_market.h>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 0.00001;

go_bandit([]{
describe("Market", []{
    Market::TradedPairs traded_pairs{
        Market::CP{new Currency{"GBP", "USD", 2ll, 5000, 60'000'000'000}},
        Market::CP{new Currency{"EUR", "USD", 2ll, 6000, 250'000'000'000}},
        Market::CP{new Currency{"EUR", "GBP", 2ll, 11000, 20'000'000'000}},
        Market::CP{new Currency{"USD", "BTC", 2ll, 5000, 1'000'000'000}},
    };
    Market market {traded_pairs, "USD"};

    describe("creation", [&]{
        it("determines how many 'real' currency pairs there are", [&]{
            AssertThat(market.num_real_currencies(), Equals(3));
        });
    });

    describe("querying", [&]{
        it("returns a quote for the given ccy pair", [&]{
            auto quote = market.get_quote("GBP", "EUR");
            AssertThat(quote.ccyPair, Equals("EURGBP"));
        });

        it("throws when a quote for the given ccy pair is not found", [&]{
            AssertThrows(std::runtime_error, market.get_quote("ABC","DEF"));
        });

        it("returns all quotes", [&]{
            auto quotes = market.get_all_quotes();
            AssertThat(quotes.size(), Equals(4));
            AssertThat(quotes[3].ccyPair, Equals("EURGBP"));
        });

        it("returns a triangular vector of quotes from a ccy pair to the real ccy", [&]{
            auto quotes = market.get_a_random_triangle();
            AssertThat(quotes.size(), Equals(3));
            AssertThat(quotes[0].ccyPair, Equals("EURGBP"));
            AssertThat(quotes[1].ccyPair, Equals("EURUSD"));
            AssertThat(quotes[2].ccyPair, Equals("GBPUSD"));
        });
    });

    describe("rate setting", [&]{
        it("sets the rate to whichever value is provided", [&]{
            // GIVEN & WHEN:
            market.set_rate("EUR", "GBP", 42.0);
            auto quote = market.get_quote("EUR", "GBP");
            // THEN:
            AssertThat(quote.mid, EqualsWithDelta(42.0, 0.0001));
        });
    });
});
});

