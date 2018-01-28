#pragma once

#include <exchange_currency.h>

#include <vector>
#include <array>
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

    size_t num_real_currencies() const
    {
        return num_real;
    }

    Quote get_quote(const std::string& ccy1, const std::string& ccy2) const
    {
        const auto quote = find_currency(ccy1, ccy2).quote();
        if (quote.domestic != ccy1) return quote.invert();
        return quote;
    }

    std::vector<Quote> get_all_quotes() const
    {
        std::vector<Quote> quotes;
        quotes.reserve(traded_pairs.size());
        for (const auto& ccyPair: traded_pairs) {
            quotes.push_back(ccyPair->quote());
        }
        return quotes;
    };

    std::array<Quote, 3> get_a_random_triangle() const
    {
        const size_t N = traded_pairs.size() - num_real;
        const size_t ccy_index = rand() % N + num_real;
        const auto q0 = traded_pairs[ccy_index]->quote();

        std::array<Quote, 3> quotes;
        quotes[0] = q0;
        quotes[1] = find_currency(real_ccy, q0.domestic).quote();
        quotes[2] = find_currency(real_ccy, q0.foreign).quote();

        return quotes;
    };

    void set_rate(const std::string& ccy1, const std::string& ccy2, double rate)
    {
        return find_currency(ccy1, ccy2).set_rate(rate);
    }

    double buy(const std::string& ccy1, const std::string& ccy2, double amount)
    {
        const auto& ccy_pair = find_currency(ccy1, ccy2);
        const auto quote = ccy_pair.quote();

        const bool inverted = quote.domestic != ccy1;
        if (inverted) {
            ccy_pair.sell(amount);
            return -quote.invert().bid * amount;
        }
        else {
            ccy_pair.buy(amount);
            return -quote.ask * amount;
        }
    }

    double sell(const std::string& ccy1, const std::string& ccy2, double amount)
    {
        const auto& ccy_pair = find_currency(ccy1, ccy2);
        const auto quote = ccy_pair.quote();

        const bool inverted = quote.domestic != ccy1;
        if (inverted) {
            ccy_pair.buy(amount);
            return amount * quote.invert().ask;
        }
        else {
            ccy_pair.sell(amount);
            return amount * quote.bid;
        }
    }

private:
    const Currency& find_currency(const std::string& ccy1, const std::string& ccy2) const
    {
        auto it = find_if(begin(traded_pairs), end(traded_pairs), [&](const auto& ccyPair) {
            return ccyPair->is(ccy1, ccy2);
        });
        if (it == end(traded_pairs)) throw std::runtime_error("Ccy pair not found!");
        return **it; // hurray for two-star developers...
    }
};

} // namespace exchange

