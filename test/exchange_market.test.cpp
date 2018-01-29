#include <bandit/bandit.h>

#include <exchange_market.h>
#include <set>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 0.0001;

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
            AssertThat(market.num_real_currencies(), Equals(3ul));
        });
    });

    describe("querying", [&]{
        it("returns a quote for the given ccy pair in a requested convention", [&]{
            auto quote = market.get_quote("GBP", "EUR");
            AssertThat(quote.ccy_pair, Equals("GBPEUR"));
        });

        it("throws when a quote for the given ccy pair is not found", [&]{
            AssertThrows(std::runtime_error, market.get_quote("ABC","DEF"));
        });

        it("returns all quotes", [&]{
            auto quotes = market.get_all_quotes();
            AssertThat(quotes.size(), Equals(4ul));
            AssertThat(quotes[3].ccy_pair, Equals("EURGBP"));
        });

        it("returns a triangle of quotes from a ccy pair to the real ccy", [&]{
            auto quotes = market.get_a_random_triangle();
            AssertThat(quotes.size(), Equals(3ul));
            AssertThat(quotes[0].ccy_pair, Equals("EURGBP"));
            AssertThat(quotes[1].ccy_pair, Equals("EURUSD"));
            AssertThat(quotes[2].ccy_pair, Equals("GBPUSD"));
        });
        
        it("can randomly return a quote for every traded pair", [&]{
            // GIVEN:
            std::set<std::string> quotes;
            // WHEN:
            for (size_t i = 0; i < 100; ++i) {
                quotes.insert(market.get_a_random_quote().ccy_pair);
            }
            // THEN:
            AssertThat(quotes.size(), Equals(4ul));
        });
    });

    describe("rate setting", [&]{
        it("sets the rate to whichever value is provided", [&]{
            // GIVEN & WHEN:
            market.set_rate("EUR", "GBP", 42.0);
            auto quote = market.get_quote("EUR", "GBP");
            // THEN:
            AssertThat(quote.mid, EqualsWithDelta(42.0, EPS));
        });
    });

    describe("trading", [&]{
        before_each([&]{
            market.set_rate("EUR", "USD", 0.6);
            market.set_rate("GBP", "USD", 0.5);
        });

        it("returns how much the trade has costed when buying", [&]{
            AssertThat(market.buy("EUR", "USD", 10), EqualsWithDelta(-6.001, EPS));
            AssertThat(market.buy("USD", "GBP", 10), EqualsWithDelta(-20.004, EPS));
        });

        it("returns how much the trade has costed when selling", [&]{
            AssertThat(market.sell("EUR", "USD", 10), EqualsWithDelta(5.999, EPS));
            AssertThat(market.sell("USD", "GBP", 10), EqualsWithDelta(19.996, EPS));
        });

        it("moves the market in the right direction when buying", [&]{
            // GIVEN:
            const double midEURUSD = market.get_quote("EUR", "USD").mid;
            const double midUSDGBP = market.get_quote("USD", "GBP").mid;
            // WHEN:
            market.buy("EUR", "USD", 10'000'000);
            market.buy("USD", "GBP", 10'000'000);
            // THEN:
            AssertThat(market.get_quote("EUR", "USD").mid, IsGreaterThan(midEURUSD));
            AssertThat(market.get_quote("USD", "GBP").mid, IsGreaterThan(midUSDGBP));
        });

        it("moves the market in the right direction when selling", [&]{
            // GIVEN:
            const double midEURUSD = market.get_quote("EUR", "USD").mid;
            const double midUSDGBP = market.get_quote("USD", "GBP").mid;
            // WHEN:
            market.sell("EUR", "USD", 10'000'000);
            market.sell("USD", "GBP", 10'000'000);
            // THEN:
            AssertThat(market.get_quote("EUR", "USD").mid, IsLessThan(midEURUSD));
            AssertThat(market.get_quote("USD", "GBP").mid, IsLessThan(midUSDGBP));
        });

        it("buying and then selling will leave you with less cash", [&]{
            // GIVEN:
            const double costEUR = market.buy("USD", "EUR", 1000);
            const double costUSD = market.buy("GBP", "USD", 1000);
            // WHEN:
            const double gainEUR = market.sell("USD", "EUR", 1000);
            const double gainUSD = market.sell("GBP", "USD", 1000);
            // THEN:
            AssertThat(costEUR + gainEUR, IsLessThan(0));
            AssertThat(costUSD + gainUSD, IsLessThan(0));
        });

        it("selling and then buying will also leave you with less cash", [&]{
            // GIVEN:
            const double gainUSD = market.sell("EUR", "USD", 1000);
            const double gainGBP = market.sell("USD", "GBP", 1000);
            // WHEN:
            const double costUSD = market.buy("EUR", "USD", 1000);
            const double costGBP = market.buy("USD", "GBP", 1000);
            // THEN:
            AssertThat(costUSD + gainUSD, IsLessThan(0));
            AssertThat(costGBP + gainGBP, IsLessThan(0));
        });

        it("selling is equivalent to buying the inverted ccy pair", [&]{
            // GIVEN:
            const double init_amount = 1000.0;
            // WHEN:
            const double gainUSD = market.sell("EUR", "USD", init_amount);
            const double costEUR = market.buy("USD", "EUR", gainUSD);
            // THEN:
            AssertThat(init_amount, EqualsWithDelta(-costEUR, EPS));
        });

        it("selling inverted is equivalent to buying the market ccy pair", [&]{
            // GIVEN:
            const double init_amount = 1000.0;
            // WHEN:
            const double gainGBP = market.sell("USD", "GBP", init_amount);
            const double costUSD = market.buy("GBP", "USD", gainGBP);
            // THEN:
            AssertThat(init_amount, EqualsWithDelta(-costUSD, EPS));
        });

        it("buying is equivalent to selling the inverted ccy pair", [&]{
            // GIVEN:
            const double init_amount = 1000.0;
            // WHEN:
            const double costEUR = market.buy("USD", "EUR", init_amount);
            const double gainUSD = market.sell("EUR", "USD", -costEUR);
            // THEN:
            AssertThat(init_amount, EqualsWithDelta(gainUSD, EPS));
        });

        it("buying inverted is equivalent to selling the market ccy pair", [&]{
            // GIVEN:
            const double init_amount = 1000.0;
            // WHEN:
            const double costUSD = market.buy("GBP", "USD", init_amount);
            const double gainGBP = market.sell("USD", "GBP", -costUSD);
            // THEN:
            AssertThat(init_amount, EqualsWithDelta(gainGBP, EPS));
        });
    });
});
});

