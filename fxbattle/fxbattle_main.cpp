#include <crow/crow_all.h>

#include <exchange_market.h>
#include <exchange_randomtrader.h>
#include <exchange_arbitragedestroyer.h>
#include <thread>
#include <chrono>

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

    RandomTrader trader {market, 3'000'000, 7'000'000};
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

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        crow::mustache::context x;
    
        auto page = crow::mustache::load("index.html");
        return page.render(x);
    });

    CROW_ROUTE(app, "/market")([&]{
        crow::json::wvalue x;
        const auto quotes = market.get_all_quotes();
        for (const auto& quote: quotes) {
            x[quote.ccy_pair] = quote.to_string();
        }
        return x;
    });

    app.port(18080).multithreaded().run();
}

