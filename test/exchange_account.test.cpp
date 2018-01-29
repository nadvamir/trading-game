#include <bandit/bandit.h>

#include <exchange_account.h>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 1e-4;

go_bandit([]{
describe("Account", []{
    Market::TradedPairs traded_pairs{
        Market::CP{new Currency{"GBP", "USD", 2ll, 5000, 60'000'000'000}},
        Market::CP{new Currency{"EUR", "USD", 2ll, 2000, 250'000'000'000}},
        Market::CP{new Currency{"EUR", "GBP", 2ll, 4000, 20'000'000'000}},
    };
    Market market {traded_pairs, "USD"};

    it("has a visible name", [&]{
        Account account {"Kondratiy", {}, market};
        AssertThat(account.get_name(), Equals("Kondratiy"));
    });

    it("can tell its value in any currency", [&]{
        // GIVEN:
        Account account {"Potap", {
            {"EUR", 1000.0}, {"GBP", 2000.0}, {"USD", 3000}
        }, market};
        // WHEN:
        const double worthGBP = account.get_value_in("GBP");
        const double worthEUR = account.get_value_in("EUR");
        const double worthUSD = account.get_value_in("USD");
        // THEN:
        AssertThat(worthGBP, EqualsWithDelta(8400, EPS));
        AssertThat(worthEUR, EqualsWithDelta(21000, EPS));
        AssertThat(worthUSD, EqualsWithDelta(4200, EPS));
    });
});
});

