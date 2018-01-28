#include <bandit/bandit.h>

#include <exchange_fxconverter.h>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 1e-4;

go_bandit([]{
describe("FX Converter", []{
    Market::TradedPairs traded_pairs {
        Market::CP{new Currency{"GBP", "USD", 2ll, 14400, 60'000'000'000}},
        Market::CP{new Currency{"EUR", "USD", 2ll, 12300, 250'000'000'000}},
        Market::CP{new Currency{"EUR", "GBP", 2ll, 8542, 20'000'000'000}},
        Market::CP{new Currency{"USD", "BTC", 2ll, 5000, 1'000'000'000}},
    };
    Market market {traded_pairs, "USD"};
    FXConverter converter {market};

    it("converts between currencies", [&] {
        AssertThat(converter.convert(10, "GBP", "USD"), EqualsWithDelta(14.4, EPS));
        AssertThat(converter.convert(10, "USD", "GBP"), EqualsWithDelta(6.9444, EPS));
        AssertThat(converter.convert(10, "EUR", "USD"), EqualsWithDelta(12.3, EPS));
        AssertThat(converter.convert(10, "USD", "EUR"), EqualsWithDelta(8.13, EPS));
        AssertThat(converter.convert(10, "EUR", "GBP"), EqualsWithDelta(8.542, EPS));
        AssertThat(converter.convert(10, "GBP", "EUR"), EqualsWithDelta(11.7069, EPS));
    });

    it("returns the original amount when from == to", [&] {
        AssertThat(converter.convert(10, "GBP", "GBP"), EqualsWithDelta(10.0, EPS));
    });

    it("throws when asked for a non-existing ccy pair", [&] {
        AssertThrows(std::runtime_error, converter.convert(10, "GBP", "BTC"));
    });
});
});

