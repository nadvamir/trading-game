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
    mutable std::mutex balance_mutex;

    static constexpr double overdraft {10'000.0};

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

    std::string get_name() const
    {
        return name;
    }

    std::map<std::string, double> get_holdings() const
    {
        std::lock_guard<std::mutex> guard(balance_mutex);
        return balances;
    }

    double get_value_in(const std::string& reporting_ccy) const
    {
        std::lock_guard<std::mutex> guard(balance_mutex);
        double value = 0.0;
        for (const auto& [ccy, balance]: balances) {
            value += converter.convert(balance, ccy, reporting_ccy);
        }
        return value;
    }

    void buy(
            double amount,
            const std::string& ccy_buy,
            const std::string& ccy_sell,
            double fee,
            const std::string& ccy_fee)
    {
        if (amount <= 0) {
            throw std::runtime_error("Invalid amount");
        }
        const double price_est = converter.convert(amount, ccy_buy, ccy_sell);
        const double fee_in_ccy_buy = converter.convert(fee, ccy_fee, ccy_buy);

        std::lock_guard<std::mutex> guard(balance_mutex);
        if (price_est > balances[ccy_sell] + overdraft) {
            throw std::runtime_error("Not enough "+ccy_sell+" for the transaction!");
        }

        const double price = market.buy(ccy_buy, ccy_sell, amount);
        balances[ccy_buy] += amount - fee_in_ccy_buy;
        balances[ccy_sell] += price;
    }

    void sell(
            double amount,
            const std::string& ccy_sell,
            const std::string& ccy_buy,
            double fee,
            const std::string& ccy_fee)
    {
        if (amount <= 0) {
            throw std::runtime_error("Invalid amount");
        }
        const double fee_in_ccy_buy = converter.convert(fee, ccy_fee, ccy_buy);

        std::lock_guard<std::mutex> guard(balance_mutex);
        if (balances[ccy_sell] < 0 || amount > balances[ccy_sell] + overdraft) {
            throw std::runtime_error("Not enough "+ccy_sell+" for the transaction!");
        }

        const double gain = market.sell(ccy_sell, ccy_buy, amount);
        balances[ccy_buy] += gain - fee_in_ccy_buy;
        balances[ccy_sell] -= amount;
    }
};
} // namespace exchange

