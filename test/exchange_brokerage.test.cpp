#include <bandit/bandit.h>

#include <exchange_brokerage.h>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 1e-4;

go_bandit([]{
describe("Brokerage", []{
    Market::TradedPairs traded_pairs{
        Market::CP{new Currency{"GBP", "USD", 2ll, 5000, 60'000'000'000}},
        Market::CP{new Currency{"EUR", "USD", 2ll, 2000, 250'000'000'000}},
        Market::CP{new Currency{"EUR", "GBP", 2ll, 4000, 20'000'000'000}},
    };
    Market market {traded_pairs, "USD"};
    Brokerage brokerage {{
        {"api_key1", Brokerage::A{new Account{"Kondratiy", {{"GBP", 1000.0}}, market}}},
        {"api_key2", Brokerage::A{new Account{"Potap", {{"USD", 1000.0}}, market}}},
    }, market};

    it("provides a list of accounts it manages", [&]{
        // GIVEN & WHEN:
        const auto accounts = brokerage.accounts_under_management("GBP");
        // THEN:
        const std::map<std::string, double> expected {
            {"Kondratiy", 1000.0}, {"Potap", 2000.0}
        };
        AssertThat(accounts, Equals(expected));
    });
});
});


