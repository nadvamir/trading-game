#include <fxbattle_configuration.h>
#include <crow/crow_all.h>

#include <exchange_market.h>
#include <exchange_brokerage.h>
#include <fstream>
#include <iterator>

namespace fxbattle {
namespace {
std::string read_file(std::string filename)
{
    std::ifstream fr(filename);
    std::istreambuf_iterator<char> begin(fr), end;
    std::string file_contents(begin, end);
    return file_contents;
}

auto get_traded_pairs(std::string filename)
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

auto get_holdings(const crow::json::rvalue& json_balances)
{
    std::map<std::string, double> holdings;
    for (const auto& account: json_balances) {
        holdings.insert({account.key(), account.d()});
    }
    return holdings;
}

auto get_accounts(
        std::string filename,
        exchange::Market& market)
{
    using namespace exchange;
    std::map<std::string, Brokerage::A> accounts;

    auto traders = get_config(filename);

    for (const auto& trader: traders) {
        std::string api_key = trader.key();
        std::string name = trader["name"].s();
        accounts.insert({api_key, Brokerage::A {
            new Account {name, get_holdings(trader["holdings"]), market}
        }});
    }
    return accounts;
}

} // namespace anonymous

//------------------------------------------------------------------------------
crow::json::rvalue get_config(std::string filename)
{
    return crow::json::load(read_file(filename));
}

exchange::Market get_market(std::string filename)
{
    return exchange::Market {get_traded_pairs(filename), "USD"};
}

exchange::Brokerage get_brokerage(
        std::string filename,
        const crow::json::rvalue& config,
        exchange::Market& market)
{
    return exchange::Brokerage {
        get_accounts(filename, market),
        config["fee_amount"].d(),
        config["currency"].s()
    };
}

} // namespace fxbattle

