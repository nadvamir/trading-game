#pragma once

#include <exchange_currency.h>

#include <vector>
#include <memory>
#include <algorithm>
#include <stdexcept>

namespace exchange {

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

    Quote get_quote(const std::string& ccy1, const std::string& ccy2)
    {
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

        std::vector<Quote> quotes;
        quotes.push_back(q0);
        quotes.push_back(get_quote(real_ccy, q0.domestic));
        quotes.push_back(get_quote(real_ccy, q0.foreign));

        return quotes;
    };

    void set_rate(const std::string& ccy1, const std::string& ccy2, double rate)
    {
        auto it = find_if(begin(traded_pairs), end(traded_pairs), [&](const auto& ccyPair) {
            return ccyPair->is(ccy1, ccy2);
        });
        if (it == end(traded_pairs)) throw std::runtime_error("Ccy pair not found!");
        (*it)->set_rate(rate);
    }
};

} // namespace exchange

