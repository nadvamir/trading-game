#include <crow/crow_all.h>

#include <exchange_market.h>
#include <exchange_randomtrader.h>
#include <exchange_arbitragedestroyer.h>
#include <exchange_brokerage.h>
#include <fxbattle_configuration.h>
#include <fxbattle_cachelessjsonresponse.h>
#include <thread>
#include <chrono>

using namespace fxbattle;

void save(const exchange::Brokerage& brokerage, std::string filename)
{
    crow::json::wvalue traders;
    const auto accounts = brokerage.account_names();
    for (const auto& [api_key, name]: accounts) {
        traders[api_key]["name"] = name;
        for (const auto& [ccy, amount]: brokerage.get_holdings(api_key)) {
            traders[api_key]["holdings"][ccy] = amount;
        }

    }
    std::ofstream fw(filename);
    fw << crow::json::dump(traders) << std::endl;
    fw.close();
}

int main(int argc, const char* argv[])
{
    using namespace exchange;
    using namespace std::chrono_literals;

    if (argc != 3) {
        std::cout << "Usage: fxbattle.exe traded_pairs.json traders.json" << std::endl;
        return 1;
    }

    const auto config = get_config("config.json");
    Market market = get_market(argv[1]);
    Brokerage brokerage = get_brokerage(argv[2], config, market);

    //--------------------------------------------------------------------------
    RandomTrader trader {market, config["mover_trade_size"].i()};
    std::thread market_mover([&]{
        while (true) {
            trader.trade();
            auto interval = std::chrono::milliseconds(config["trade_interval"].i());
            std::this_thread::sleep_for(interval);
        }
    });

    RandomTrader chaos_trader {market, config["chaos_trade_size"].i()};
    std::thread chaos_mover([&]{
        while (true) {
            chaos_trader.trade();
            auto interval = std::chrono::milliseconds(config["chaos_interval"].i());
            std::this_thread::sleep_for(interval);
        }
    });

    std::thread arbitrage_destroyer([&]{
        while (true) {
            ArbitrageDestroyer::normalise(market);
            auto interval = std::chrono::milliseconds(config["arbitrage_interval"].i());
            std::this_thread::sleep_for(interval);
        }
    });

    std::thread saver([&]{
        while (true) {
            save(brokerage, config["save_file"].s());
            auto interval = std::chrono::seconds(config["saving_interval"].i());
            std::this_thread::sleep_for(interval);
        }
    });

    //--------------------------------------------------------------------------
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        crow::mustache::context x;
    
        auto page = crow::mustache::load("index.html");
        return page.render(x);
    });

    CROW_ROUTE(app, "/market")([&market]{
        crow::json::wvalue x;
        const auto quotes = market.get_all_quotes();
        for (const auto& quote: quotes) {
            x[quote.ccy_pair] = quote.to_string();
        }
        return CachelessJsonResponse{x};
    });

    CROW_ROUTE(app, "/accounts")([&brokerage]{
        crow::json::wvalue x;
        const auto accounts = brokerage.accounts_under_management("GBP");
        for (const auto& [name, balance]: accounts) {
            x[name] = round(balance);
        }
        return CachelessJsonResponse{x};
    });

    CROW_ROUTE(app, "/account/<string>")([&brokerage](const auto& api_key){
        crow::json::wvalue x;
        try {
            const auto holdings = brokerage.get_holdings(api_key);
            for (const auto& [ccy, balance]: holdings) {
                x[ccy] = balance;
            }
        }
        catch (std::runtime_error& e) {
            x["error"] = e.what();
        }
        return CachelessJsonResponse{x};
    });

    CROW_ROUTE(app, "/trade/<string>/<string>/<string>/<double>")
    ([&brokerage](const auto& api_key,
                  const auto& direction,
                  const auto& ccy_pair,
                  double amount) {
        crow::json::wvalue x;
        try {
            if (ccy_pair.size() != 6) throw std::runtime_error("Wrong ccy pair");
            const auto ccy1 = ccy_pair.substr(0, 3);
            const auto ccy2 = ccy_pair.substr(3);

            const auto holdings = (direction == "buy")
                                ? brokerage.buy(api_key, amount, ccy1, ccy2)
                                : brokerage.sell(api_key, amount, ccy1, ccy2);

            for (const auto& [ccy, balance]: holdings) {
                x[ccy] = balance;
            }
        }
        catch (std::runtime_error& e) {
            x["error"] = e.what();
        }
        return CachelessJsonResponse{x};
    });

    app.port(config["port"].i()).multithreaded().run();
}

