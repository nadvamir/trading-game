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
    Market& market;

public:
    Brokerage(std::map<std::string, A> accounts, Market& market)
        : accounts(accounts), market(market)
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
};
} // namespace exchange

