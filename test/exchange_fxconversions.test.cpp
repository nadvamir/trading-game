#include <bandit/bandit.h>
#include <exchange_fxconversions.h>

using namespace snowhouse;
using namespace bandit;
using namespace exchange;

const double EPS = 0.00001;

go_bandit([]{
describe("FX Conversions", []{
    describe("bp_to_double", []{
        it("is a div by 10,000 conversion", []{
            AssertThat(bp_to_double(12345), EqualsWithDelta(1.2345, EPS));
        });
    });

    describe("double_to_bp", []{
        it("is a mult * 10,000 rounding conversion", []{
            AssertThat(double_to_bp(1.23451), Equals(12345));
            AssertThat(double_to_bp(1.23456), Equals(12346));
        });
    });
});
});

