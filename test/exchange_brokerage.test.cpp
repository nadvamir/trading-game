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
    }, 10, "GBP"};

    describe("Reporting", [&]{
        it("provides a list of values of accounts it manages", [&]{
            // GIVEN & WHEN:
            const auto accounts = brokerage.accounts_under_management("GBP");
            // THEN:
            const std::map<std::string, double> expected {
                {"Kondratiy", 1000.0}, {"Potap", 2000.0}
            };
            AssertThat(accounts, Equals(expected));
        });

        it("provides a mapping from api key to an account name", [&]{
            // GIVEN & WHEN:
            const auto accounts = brokerage.account_names();
            // THEN:
            const std::map<std::string, std::string> expected {
                {"api_key1", "Kondratiy"}, {"api_key2", "Potap"}
            };
            AssertThat(accounts, Equals(expected));
        });

        it("provides a detailed breakdown for each account", [&]{
            // GIVEN & WHEN:
            auto holdings = brokerage.get_holdings("api_key1");
            // THEN
            AssertThat(holdings["GBP"], EqualsWithDelta(1000.0, EPS));
        });

        it("throws when an api_key is not found", [&]{
            AssertThrows(std::runtime_error, brokerage.get_holdings("fake_api"));
        });
    });

    describe("Trading", [&]{
        it("throws when an api_key is not found", [&]{
            AssertThrows(std::runtime_error,
                         brokerage.buy("fake_api", 1, "EUR", "USD"));
            AssertThrows(std::runtime_error,
                         brokerage.sell("fake_api", 1, "EUR", "USD"));
        });

        // it("throws when trying to buy or sell an invalid pair", [&]{
        //     try {
        //         brokerage.buy("api_key1", 1, "XYZ", "GBP");
        //     }
        //     catch(std::exception& e) {
        //         std::cout << e.what() << std::endl;
        //     }

        //     AssertThrows(std::runtime_error,
        //                  brokerage.buy("api_key1", 1, "XYZ", "GBP"));
        //     AssertThrows(std::runtime_error,
        //                  brokerage.sell("api_key1", 1, "GBP", "XYZ"));

        //     auto holdings = brokerage.get_holdings("api_key1");
        //     AssertThat(holdings.find("XYZ"), Equals(holdings.end()));
        // });

        it("applies a commission of 10GBP to each trade", [&]{
            // GIVEN:
            Brokerage brokerage2 {{
                {"api_key1", Brokerage::A{new Account{
                    "Kondratiy", {{"GBP", 1000.0}}, market
                }}},
            }, 10, "GBP"};
            // WHEN:
            auto holdings1 = brokerage2.sell("api_key1", 200, "GBP", "USD");
            auto holdings2 = brokerage2.buy("api_key1", 100, "EUR", "USD");
            // THEN:
            AssertThat(holdings1["GBP"], EqualsWithDelta(800, EPS)); // sell
            AssertThat(holdings1["USD"], EqualsWithDelta(94.98, EPS)); // buy
            AssertThat(holdings2["EUR"], EqualsWithDelta(75, EPS)); // buy
            AssertThat(holdings2["USD"], EqualsWithDelta(74.97, EPS)); // sell

        });
    });
});
});


