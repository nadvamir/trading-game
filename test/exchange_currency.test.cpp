#include <bandit/bandit.h>

#include <exchange_currency.h>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 0.00001;

go_bandit([]{
describe("Currency", []{
    describe("Quote", []{
        Currency GBPUSD {"GBP", "USD", 4ll, 14401, 1000};
        Currency EURGBP {"EUR", "GBP", 2ll, 8300};

        it("combines currencies to form one ccy pair description", [&]{
            AssertThat(GBPUSD.quote().ccy_pair, Equals("GBPUSD"));
            AssertThat(EURGBP.quote().ccy_pair, Equals("EURGBP"));
        });

        it("returns the volume and spread", [&]{
            AssertThat(GBPUSD.quote().volume, Equals(1000));
            AssertThat(GBPUSD.quote().spread, Equals(4));
        });

        it("applies the spread to the quote", [&]{
            Quote qGBPUSD = GBPUSD.quote();
            Quote qEURGBP = EURGBP.quote();
            AssertThat(qGBPUSD.ask - qGBPUSD.bid, EqualsWithDelta(0.0004, EPS));
            AssertThat(qEURGBP.ask - qEURGBP.bid, EqualsWithDelta(0.0002, EPS));
        });

        it("provides mid for convenience", [&]{
            Quote qGBPUSD = GBPUSD.quote();
            AssertThat(qGBPUSD.mid, IsGreaterThan(qGBPUSD.bid));
            AssertThat(qGBPUSD.mid, IsLessThan(qGBPUSD.ask));
        });

        it("serialises with the correct precision", [&]{
            std::stringstream ss;
            ss << GBPUSD.quote();
            AssertThat(ss.str(), Equals("GBPUSD 1.4399 1.4403"));
            AssertThat(ss.str(), Equals(GBPUSD.quote().to_string()));
        });

        it("returns an inverted quote if need be", [&]{
            // GIVEN:
            Quote qGBPUSD= GBPUSD.quote();
            // WHEN:
            Quote qUSDGBP = qGBPUSD.invert();
            // THEN:
            AssertThat(qUSDGBP.ccy_pair, Equals("USDGBP"));
            AssertThat(qUSDGBP.domestic, Equals("USD"));
            AssertThat(qUSDGBP.foreign, Equals("GBP"));
            AssertThat(qUSDGBP.bid, EqualsWithDelta(1/qGBPUSD.bid, EPS));
            AssertThat(qUSDGBP.mid, EqualsWithDelta(1/qGBPUSD.mid, EPS));
            AssertThat(qUSDGBP.ask, EqualsWithDelta(1/qGBPUSD.ask, EPS));
        });
    });

    describe("Trading", []{
        it("increases the rate when you buy", []{
            // GIVEN:
            Currency EURGBP {"EUR", "GBP", 2ll, 5000ll};
            // WHEN:
            EURGBP.buy(1'000'000'0000);
            // THEN:
            AssertThat(EURGBP.quote().mid, IsGreaterThan(0.5));
        });

        it("decreases the rate when you sell", []{
            // GIVEN:
            Currency EURGBP {"EUR", "GBP", 2ll, 5000ll};
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
            Currency EURGBP {"EUR", "GBP", 2ll, initRate, eurVolume};
            Currency GBPUSD {"GBP", "USD", 2ll, initRate, usdVolume};
            // WHEN:
            EURGBP.sell(10'000'000);
            GBPUSD.sell(10'000'000);
            // THEN:
            const double largerMove = 0.5 - GBPUSD.quote().mid;
            const double smallerMove = 0.5 - EURGBP.quote().mid;
            AssertThat(smallerMove, IsLessThan(largerMove));
        });

        it("alters the rate within reasonable limits", [&]{
            // GIVEN:
            const long long eurVolume = 10'000'000'000;
            Currency EURGBP {"EUR", "GBP", 2ll, 10000, eurVolume};
            // WHEN:
            EURGBP.buy(10'000'000);
            // THEN:
            const double move = EURGBP.quote().mid - 1.0;
            AssertThat(move, IsLessThan(0.001));
            AssertThat(move, IsGreaterThan(0.0001));
        });

        it("moves the market downwards more if the price went too hight", [&]{
            // GIVEN:
            const long long rate = 10000;
            Currency EURGBP {"EUR", "GBP", 2ll, rate, 10'000'000'000};

            // WHEN:
            EURGBP.set_rate(2.0);
            EURGBP.buy(10'000'000);
            const double moveUp = EURGBP.quote().mid - 2.0;
            EURGBP.set_rate(2.0);
            EURGBP.sell(10'000'000);
            const double moveDown = -(EURGBP.quote().mid - 2.0);

            // THEN:
            AssertThat(moveUp + EPS, IsLessThan(moveDown));
        });

        it("moves the market upwards more if the price went too low", [&]{
            // GIVEN:
            const long long rate = 10000;
            Currency EURGBP {"EUR", "GBP", 2ll, rate, 10'000'000'000};

            // WHEN:
            EURGBP.set_rate(0.5);
            EURGBP.buy(10'000'000);
            const double moveUp = EURGBP.quote().mid - 0.5;
            EURGBP.set_rate(0.5);
            EURGBP.sell(10'000'000);
            const double moveDown = -(EURGBP.quote().mid - 0.5);

            // THEN:
            AssertThat(moveDown + EPS, IsLessThan(moveUp));
        });

        it("moves the market by at least 1bp with a 1M trade regardless of volume", [&]{
            // GIVEN:
            const long long eurVolume = 10'000'000'000'000;
            Currency EURGBP {"EUR", "GBP", 2ll, 10000, eurVolume};
            // WHEN:
            EURGBP.buy(10'000'000);
            // THEN:
            const double move = EURGBP.quote().mid - 1.0;
            AssertThat(move, IsGreaterThan(0.00009));
        });
    });

    describe("Rebalancing", []{
        it("overwrites the current rate", []{
            // GIVEN:
            Currency EURGBP {"EUR", "GBP", 2ll, 5000ll};
            // WHEN:
            EURGBP.set_rate(0.7);
            // THEN:
            AssertThat(EURGBP.quote().mid, EqualsWithDelta(0.7, EPS));
        });
    });

    describe("Matching", []{
        Currency EURGBP {"EUR", "GBP", 2ll, 5000ll};
        it("can tell whether the currency pair has requested currency", [&]{
            AssertThat(EURGBP.is("EUR"), Equals(true));
            AssertThat(EURGBP.is("GBP"), Equals(true));
            AssertThat(EURGBP.is("USD"), Equals(false));
        });

        it("can tell what currency pair it is regardless of quoting conventions", [&]{
            AssertThat(EURGBP.is("EUR", "GBP"), Equals(true));
            AssertThat(EURGBP.is("GBP", "EUR"), Equals(true));
            AssertThat(EURGBP.is("USD", "GBP"), Equals(false));
        });
    });
});
});

