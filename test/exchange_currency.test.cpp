#include <bandit/bandit.h>

#include <exchange_currency.h>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 0.00001;

go_bandit([]{
describe("Currency", []{
    describe("Quote", []{
        Currency USDGBP {"USD", "GBP", true, 4ll, 6944ll};
        Currency EURGBP {"EUR", "GBP", false, 2ll, 8300ll};

        it("follows market convention for naming quotes", [&]{
            AssertThat(USDGBP.quote().ccyPair, Equals("GBPUSD"));
            AssertThat(EURGBP.quote().ccyPair, Equals("EURGBP"));
        });

        it("follows market convention for quoting rate", [&]{
            AssertThat(USDGBP.quote().bid, IsGreaterThan(1));
            AssertThat(EURGBP.quote().bid, IsLessThan(1));
        });

        it("applies the spread to the quote", [&]{
            Quote qUSDGBP = USDGBP.quote();
            Quote qEURGBP = EURGBP.quote();
            AssertThat(qUSDGBP.ask - qUSDGBP.bid, EqualsWithDelta(0.0004, EPS));
            AssertThat(qEURGBP.ask - qEURGBP.bid, EqualsWithDelta(0.0002, EPS));
        });

        it("provides mid for convenience", [&]{
            Quote qUSDGBP = USDGBP.quote();
            AssertThat(qUSDGBP.mid, IsGreaterThan(qUSDGBP.bid));
            AssertThat(qUSDGBP.mid, IsLessThan(qUSDGBP.ask));
        });

        it("serialises with the correct precision", [&]{
            std::stringstream ss;
            ss << USDGBP.quote();
            AssertThat(ss.str(), Equals("GBPUSD 1.4399 1.4403"));
        });
    });

    describe("Trading", []{
        it("increases the rate when you buy", []{
            // GIVEN:
            Currency EURGBP {"EUR", "GBP", false, 2ll, 5000ll};
            // WHEN:
            EURGBP.buy(1'000'000'0000);
            // THEN:
            AssertThat(EURGBP.quote().mid, IsGreaterThan(0.5));
        });

        it("decreases the rate when you sell", []{
            // GIVEN:
            Currency EURGBP {"EUR", "GBP", false, 2ll, 5000ll};
            // WHEN:
            EURGBP.sell(1'000'000'0000);
            // THEN:
            AssertThat(EURGBP.quote().mid, IsLessThan(0.5));
        });

        it("alters the rate more when trade volume is smaller", []{
            // GIVEN:
            const long long eurVolume = 10'000'000'000;
            const long long usdVolume = 3'000'000'000;
            const long long initRate = 5000;
            Currency EURGBP {"EUR", "GBP", false, 2ll, initRate, eurVolume};
            Currency USDGBP {"USD", "GBP", false, 2ll, initRate, usdVolume};
            // WHEN:
            EURGBP.sell(1'000'000);
            USDGBP.sell(1'000'000);
            // THEN:
            const double largerMove = 0.5 - USDGBP.quote().mid;
            const double smallerMove = 0.5 - EURGBP.quote().mid;
            AssertThat(smallerMove, IsLessThan(largerMove));
        });
    });

    describe("Rebalancing", []{
        it("overwrites the current rate", []{
            // GIVEN:
            Currency EURGBP {"EUR", "GBP", false, 2ll, 5000ll};
            // WHEN:
            EURGBP.set_rate(0.7);
            // THEN:
            AssertThat(EURGBP.quote().mid, EqualsWithDelta(0.7, EPS));
        });
    });
});
});

