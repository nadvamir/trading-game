#pragma once

#include <crow/crow_all.h>

#include <exchange_market.h>
#include <exchange_brokerage.h>
#include <fstream>
#include <iterator>

namespace fxbattle {

inline std::string read_file(std::string filename)
{
    std::ifstream fr(filename);
    std::istreambuf_iterator<char> begin(fr), end;
    std::string file_contents(begin, end);
    return file_contents;
}

inline auto get_config(std::string filename)
{
    return crow::json::load(read_file(filename));
}

inline auto get_traded_pairs(std::string filename)
{
    using namespace exchange;
    Market::TradedPairs traded_pairs;

    auto pairs = get_config(filename);
    for (const auto& ccy_pair: pairs) {
        std::string ccys = ccy_pair.key();

        traded_pairs.push_back(Market::CP{new Currency{
            ccys.substr(0, 3),
            ccys.substr(3),
            ccy_pair["spread"].i(),
            double_to_bp(ccy_pair["rate"].d()),
            ccy_pair["volume"].i()
        }});
    }
    return traded_pairs;
}

inline auto get_accounts(
        std::string filename,
        const crow::json::rvalue& config,
        exchange::Market& market)
{
    using namespace exchange;
    std::map<std::string, Brokerage::A> accounts;

    auto currency = config["currency"].s();
    auto traders = get_config(filename);

    for (const auto& trader: traders) {
        std::string api_key = trader.key();
        std::string name = trader.s();
        accounts.insert({api_key, Brokerage::A {
            new Account {name, {{currency, config["init_amount"].d()}}, market}
        }});
    }
    return accounts;
}

} // namespace fxbattle

