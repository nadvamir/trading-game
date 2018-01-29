#pragma once

#include <exchange_fxconversions.h>
#include <exchange_market.h>
#include <exchange_fxconverter.h>
#include <mutex>

namespace exchange {
class Account {
    std::string name;
    std::map<std::string, double> balances;
    Market& market;
    FXConverter converter;
    std::mutex balance_mutex;

public:
    Account(
            std::string name,
            std::map<std::string, double> initial_balances,
            Market& market)
        : name(name)
        , balances(initial_balances)
        , market(market)
        , converter(market)
    {
    }

    std::string get_name() 
    {
        return name;
    }

    double get_value_in(const std::string& reporting_ccy)
    {
        std::lock_guard<std::mutex> guard(balance_mutex);
        double value = 0.0;
        for (const auto& [ccy, balance]: balances) {
            value += converter.convert(balance, ccy, reporting_ccy);
        }
        return value;
    }
};
} // namespace exchange

