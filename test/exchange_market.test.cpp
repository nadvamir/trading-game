#include <bandit/bandit.h>

#include <exchange_currency.h>
// #include <exchange_market.h>

#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 0.00001;

class Market {
public:
    using CP = std::shared_ptr<Currency>;
    using TradedPairs = std::vector<CP>;

private:
    TradedPairs traded_pairs;
    std::string real_ccy;
    size_t num_real;

public:
    Market(TradedPairs ccys, const std::string& real)
        : traded_pairs(ccys), real_ccy(real)
    {
        auto it = partition(begin(traded_pairs), end(traded_pairs), [&](const auto& ccyPair) {
            return ccyPair->is(real);
        });
        num_real = it - begin(traded_pairs);
    }

    size_t num_real_currencies()
    {
        return num_real;
    }

    Quote get_quote(const std::string& ccyPair)
    {
        const auto ccy1 = ccyPair.substr(0, 3);
        const auto ccy2 = ccyPair.substr(3);
        auto it = find_if(begin(traded_pairs), end(traded_pairs), [&](const auto& ccyPair) {
            return ccyPair->is(ccy1, ccy2);
        });
        if (it == end(traded_pairs)) throw std::runtime_error("Ccy pair not found!");
        return (*it)->quote();
    }

    std::vector<Quote> get_all_quotes()
    {
        std::vector<Quote> quotes;
        quotes.reserve(traded_pairs.size());
        for (const auto& ccyPair: traded_pairs) {
            quotes.push_back(ccyPair->quote());
        }
        return quotes;
    };

    std::vector<Quote> get_a_random_triangle()
    {
        const size_t N = traded_pairs.size() - num_real;
        const size_t ccy_index = rand() % N + num_real;
        const auto q0 = traded_pairs[ccy_index]->quote();
        const auto ccy1 = q0.ccyPair.substr(0, 3);
        const auto ccy2 = q0.ccyPair.substr(3);

        std::vector<Quote> quotes;
        quotes.push_back(q0);
        quotes.push_back(get_quote(real_ccy + ccy1));
        quotes.push_back(get_quote(real_ccy + ccy2));

        return quotes;
    };

    void set_rate(const std::string& ccyPair, double rate)
    {
        const auto ccy1 = ccyPair.substr(0, 3);
        const auto ccy2 = ccyPair.substr(3);
        
        auto it = find_if(begin(traded_pairs), end(traded_pairs), [&](const auto& ccyPair) {
            return ccyPair->is(ccy1, ccy2);
        });
        if (it == end(traded_pairs)) throw std::runtime_error("Ccy pair not found!");
        (*it)->set_rate(rate);
    }
};

go_bandit([]{
describe("Market", []{
    Market::TradedPairs traded_pairs{
        Market::CP{new Currency{"USD", "GBP", false, 2ll, 5000, 60'000'000'000}},
        Market::CP{new Currency{"USD", "EUR", false, 2ll, 6000, 250'000'000'000}},
        Market::CP{new Currency{"GBP", "EUR", false, 2ll, 11000, 20'000'000'000}},
        Market::CP{new Currency{"USD", "BTC", false, 2ll, 5000, 1'000'000'000}},
    };
    Market market {traded_pairs, "USD"};

    describe("creation", [&]{
        it("determines how many 'real' currency pairs there are", [&]{
            AssertThat(market.num_real_currencies(), Equals(3));
        });
    });

    describe("querying", [&]{
        it("returns a quote for the given ccy pair", [&]{
            auto quote = market.get_quote("EURGBP");
            AssertThat(quote.ccyPair, Equals("GBPEUR"));
        });

        it("throws when a quote for the given ccy pair is not found", [&]{
            AssertThrows(std::runtime_error, market.get_quote("ABCDEF"));
        });

        it("returns all quotes", [&]{
            auto quotes = market.get_all_quotes();
            AssertThat(quotes.size(), Equals(4));
            AssertThat(quotes[3].ccyPair, Equals("GBPEUR"));
        });

        it("returns a triangular vector of quotes from a ccy pair to the real ccy", [&]{
            auto quotes = market.get_a_random_triangle();
            AssertThat(quotes.size(), Equals(3));
            AssertThat(quotes[0].ccyPair, Equals("GBPEUR"));
            AssertThat(quotes[1].ccyPair, Equals("USDGBP"));
            AssertThat(quotes[2].ccyPair, Equals("USDEUR"));
        });
    });

    describe("rate setting", [&]{
        it("sets the rate to whichever value is provided", [&]{
            // GIVEN & WHEN:
            market.set_rate("GBPEUR", 42.0);
            auto quote = market.get_quote("GBPEUR");
            // THEN:
            AssertThat(quote.mid, EqualsWithDelta(42.0, 0.0001));
        });
    });
});
});

