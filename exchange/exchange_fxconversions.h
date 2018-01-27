#pragma once

#include <cmath>

namespace exchange {
inline double bp_to_double(long long bp) {
    return bp / 10'000.0;
}

inline long long double_to_bp(double units) {
    return round(units * 10'000);
}
} // namespace exchange

