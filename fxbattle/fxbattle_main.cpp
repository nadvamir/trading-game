#include <crow/crow_all.h>

#include <exchange_market.h>
#include <exchange_randomtrader.h>
#include <exchange_arbitragedestroyer.h>
#include <exchange_brokerage.h>
#include <thread>
#include <chrono>
#include <cmath>

int main()
{
    using namespace exchange;
    using namespace std::chrono_literals;

    Market::TradedPairs traded_pairs{
        Market::CP{new Currency{"EUR", "USD", 2ll, 12427, 250'000'000'000}},
        Market::CP{new Currency{"GBP", "USD", 4ll, 14170, 68'000'000'000}},
        Market::CP{new Currency{"USD", "JPY", 4ll, 1088139, 64'000'000'000}},
        Market::CP{new Currency{"EUR", "JPY", 6ll, 1352231, 30'000'000'000}},
        Market::CP{new Currency{"EUR", "GBP", 4ll, 8770, 20'000'000'000}},
        Market::CP{new Currency{"GBP", "JPY", 8ll, 1541926, 5'000'000'000}},
    };
    Market market {traded_pairs, "USD"};

    RandomTrader trader {market, 3'000'000};
    std::thread market_mover([&trader]{
        while (true) {
            trader.trade();
            std::this_thread::sleep_for(3ms);
        }
    });

    std::thread arbitrage_destroyer([&market]{
        while (true) {
            ArbitrageDestroyer::normalise(market);
            std::this_thread::sleep_for(5ms);
        }
    });

    Brokerage brokerage {{
        {"api_key1", Brokerage::A{new Account{"Kondratiy", {{"GBP", 10000.0}, {"USD", 10000.0}}, market}}},
        {"api_key2", Brokerage::A{new Account{"Potap", {{"GBP", 10000.0}, {"EUR", 10000.0}}, market}}},
    }, 10, "GBP"};

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
        return x;
    });

    CROW_ROUTE(app, "/accounts")([&brokerage]{
        crow::json::wvalue x;
        const auto accounts = brokerage.accounts_under_management("GBP");
        for (const auto& [name, balance]: accounts) {
            x[name] = round(balance);
        }
        return x;
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
        return x;
    });

    app.port(18080).multithreaded().run();
}

