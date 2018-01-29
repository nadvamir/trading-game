#pragma once

#include <exchange_account.h>
#include <exchange_market.h>
#include <memory>
#include <map>

namespace exchange {
class Brokerage {
public:
    using A = std::shared_ptr<Account>;
private:
    std::map<std::string, A> accounts;

public:
    Brokerage(std::map<std::string, A> accounts)
        : accounts(accounts)
    {
    }

    std::map<std::string, double> accounts_under_management(const std::string& reporting_ccy) const
    {
        std::map<std::string, double> report;
        for (const auto& [_, account]: accounts) {
            report.insert({account->get_name(), account->get_value_in(reporting_ccy)});
        }
        return report;
    }

    std::map<std::string, double> get_holdings(const std::string& api_key) const
    {
        return get_account(api_key).get_holdings();
    }

    std::map<std::string, double> buy(
            const std::string& api_key,
            double amount,
            const std::string& buy_ccy,
            const std::string& sell_ccy)
    {
        get_account(api_key).buy(amount, buy_ccy, sell_ccy, 10, "GBP");
        return get_holdings(api_key);
    }

    std::map<std::string, double> sell(
            const std::string& api_key,
            double amount,
            const std::string& sell_ccy,
            const std::string& buy_ccy)
    {
        get_account(api_key).sell(amount, sell_ccy, buy_ccy, 10, "GBP");
        return get_holdings(api_key);
    }

private:
    Account& get_account(const std::string& api_key)
    {
        auto account = accounts.find(api_key);
        if (account == end(accounts)) throw std::runtime_error("Account not found");
        return *(account->second);
    }

    const Account& get_account(const std::string& api_key) const
    {
        const auto account = accounts.find(api_key);
        if (account == end(accounts)) throw std::runtime_error("Account not found");
        return *(account->second);
    }
};
} // namespace exchange

