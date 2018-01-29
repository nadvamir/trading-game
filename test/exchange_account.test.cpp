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

    describe("Reporting", [&]{
        it("has a visible name", [&]{
            Account account {"Kondratiy", {}, market};
            AssertThat(account.get_name(), Equals("Kondratiy"));
        });

        it("can tell its holdings", [&]{
            // GIVEN:
            std::map<std::string, double> holdings {
                {"EUR", 1000.0}, {"GBP", 2000.0}, {"USD", 3000}
            };
            // WHEN:
            Account account {"Potap", holdings, market};
            // THEN:
            AssertThat(account.get_holdings(), Equals(holdings));
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

    describe("Trading", [&]{
        it("buys currency, paying a brokerage fee and spread", [&]{
            // GIVEN:
            Account account {"Potap", {
                {"EUR", 1000.0}, {"GBP", 2000.0}, {"USD", 3000}
            }, market};
            // WHEN:
            account.buy(1000, "GBP", "USD", 10, "GBP");
            // THEN:
            auto holdings = account.get_holdings();
            AssertThat(holdings["GBP"], EqualsWithDelta(2990.0, EPS));
            AssertThat(holdings["USD"], EqualsWithDelta(2499.9, EPS));
        });

        it("pays the fee in the receiving currency", [&]{
            // GIVEN:
            Account account {"Potap", {
                {"EUR", 1000.0}, {"GBP", 2000.0}, {"USD", 3000}
            }, market};
            // WHEN:
            account.buy(1000, "EUR", "USD", 10, "GBP");
            // THEN:
            auto holdings = account.get_holdings();
            AssertThat(holdings["EUR"], EqualsWithDelta(1975.0, EPS));
            AssertThat(holdings["USD"], EqualsWithDelta(2799.9, EPS));
        });

        it("throws if the account does not have enough money for the trade", [&]{
            // GIVEN:
            Account account {"Potap", {
                {"EUR", 1000.0}, {"GBP", 2000.0}, {"USD", 3000}
            }, market};
            // WHEN & THEN:
            AssertThrows(std::runtime_error, account.buy(1000, "GBP", "EUR", 1, "GBP"));
        });

        it("can get into negative as a result of execution of the trade", [&]{
            // GIVEN:
            Account account {"Potap", {
                {"EUR", 1000.0}, {"GBP", 2000.0}, {"USD", 3000}
            }, market};
            // WHEN:
            account.buy(5000, "EUR", "GBP", 10, "GBP");
            // THEN:
            auto holdings = account.get_holdings();
            AssertThat(holdings["EUR"], EqualsWithDelta(5975.0, EPS));
            AssertThat(holdings["GBP"], EqualsWithDelta(-0.5, EPS));
        });

        it("sells currency, paying a brokerage fee and spread", [&]{
            // GIVEN:
            Account account {"Potap", {
                {"EUR", 1000.0}, {"GBP", 2000.0}, {"USD", 3000}
            }, market};
            // WHEN:
            account.sell(1000, "EUR", "USD", 10, "GBP");
            // THEN:
            auto holdings = account.get_holdings();
            AssertThat(holdings["EUR"], EqualsWithDelta(0.0, EPS));
            AssertThat(holdings["USD"], EqualsWithDelta(3194.9, EPS));
        });

        it("throws when trying to sell more than one holds", [&]{
            // GIVEN:
            Account account {"Potap", {
                {"EUR", 1000.0}, {"GBP", 2000.0}, {"USD", 3000}
            }, market};
            // WHEN & THEN:
            AssertThrows(std::runtime_error, account.sell(4000, "GBP", "EUR", 1, "GBP"));
        });

        it("throws when trying to buy or sell a negative amount", [&]{
            // GIVEN:
            Account account {"Potap", {
                {"EUR", 1000.0}, {"GBP", 2000.0}, {"USD", 3000}
            }, market};
            // WHEN & THEN:
            AssertThrows(std::runtime_error, account.buy(-1, "GBP", "EUR", 1, "GBP"));
            AssertThrows(std::runtime_error, account.sell(-1, "GBP", "EUR", 1, "GBP"));
        });

    });
});
});

