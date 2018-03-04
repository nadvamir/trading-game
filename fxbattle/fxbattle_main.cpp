#include <crow/crow_all.h>

#include <exchange_market.h>
#include <exchange_randomtrader.h>
#include <exchange_arbitragedestroyer.h>
#include <exchange_brokerage.h>
#include <fxbattle_configuration.h>
#include <fxbattle_cachelessjsonresponse.h>
#include <fxbattle/fxbattle_htmlresponse.h>
#include <fxbattle/fxbattle_metrics.h>
#include <thread>
#include <atomic>

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

void save_market(const exchange::Market& market, std::string filename)
{
    crow::json::wvalue traded_pairs;
    const auto quotes = market.get_all_quotes();
    for (const auto& quote: quotes) {
        traded_pairs[quote.ccy_pair]["spread"] = quote.spread;
        traded_pairs[quote.ccy_pair]["rate"] = quote.mid;
        traded_pairs[quote.ccy_pair]["volume"] = quote.volume;
    }
    std::ofstream fw(filename);
    fw << crow::json::dump(traded_pairs) << std::endl;
    fw.close();
}

struct Intervals {
    std::atomic_int trade_interval;
    std::atomic_int chaos_interval;
    std::atomic_int arbitrage_interval;
    std::atomic_int saving_interval;
};

int main(int argc, const char* argv[])
{
    using namespace exchange;
    using namespace std::chrono_literals;

    if (argc != 4) {
        std::cout << "Usage: fxbattle.exe "
                  << "config.json traded_pairs.json traders.json" << std::endl;
        return 1;
    }
    const std::string config_file = argv[1];
    const std::string traded_pairs_file = argv[2];
    const std::string traders_file = argv[3];

    const auto config = get_config(config_file);
    Market market = get_market(traded_pairs_file);
    Brokerage brokerage = get_brokerage(traders_file, config, market);

    Intervals intervals {
        config["trade_interval"].i(),
        config["chaos_interval"].i(),
        config["arbitrage_interval"].i(),
        config["saving_interval"].i()
    };

    //--------------------------------------------------------------------------
    // Game logic
    RandomTrader trader {market, config["mover_trade_size"].i()};
    std::thread market_mover([&]{
        while (true) {
            trader.trade();
            auto interval = std::chrono::milliseconds(intervals.trade_interval);
            std::this_thread::sleep_for(interval);
        }
    });

    RandomTrader chaos_trader {market, config["chaos_trade_size"].i()};
    std::thread chaos_mover([&]{
        while (true) {
            chaos_trader.trade();
            auto interval = std::chrono::milliseconds(intervals.chaos_interval);
            std::this_thread::sleep_for(interval);
        }
    });

    std::thread arbitrage_destroyer([&]{
        while (true) {
            ArbitrageDestroyer::normalise(market);
            auto interval = std::chrono::milliseconds(intervals.arbitrage_interval);
            std::this_thread::sleep_for(interval);
        }
    });

    std::thread saver([&]{
        while (true) {
            save(brokerage, traders_file + ".sav");
            save_market(market, traded_pairs_file + ".sav");
            auto interval = std::chrono::seconds(intervals.saving_interval);
            std::this_thread::sleep_for(interval);
        }
    });

    //--------------------------------------------------------------------------
    // Trading API
    auto getEnvOrDefault = [](const char *name,
                              const char *def) -> const char * {
      char *val = std::getenv(name);
      if (!val)
        return def;
      return val;
    };

    const std::string ENV_STATSHOST =
        getEnvOrDefault("ENV_STATSHOST", "localhost");

    const int ENV_STATSPORT =
        std::atoi(getEnvOrDefault("ENV_STATSPORT", "8125"));

    statsd stats;

    if (!stats.start(ENV_STATSHOST, ENV_STATSPORT)) {
      std::cout << "not logging stats, set ENV_STATSHOST and ENV_STATSPORT to "
                   "enable\n";
    }

    std::string api_timer_stat{"api_timer"};
    std::string page_hits_stat{"page_hits"};

    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([&stats, &page_hits_stat](){
        statsd::timing_scope timer{stats, page_hits_stat};
        crow::mustache::context x;
    
        auto page = crow::mustache::load("index.html");
        return HtmlResponse{page.render(x)};
    });

    CROW_ROUTE(app, "/market")
    ([&market, &stats, &api_timer_stat]{
        statsd::timing_scope timer{stats, api_timer_stat};
        crow::json::wvalue x;
        const auto quotes = market.get_all_quotes();
        for (const auto& quote: quotes) {
            x[quote.ccy_pair] = quote.to_string();
        }
        return CachelessJsonResponse{x};
    });

    CROW_ROUTE(app, "/accounts")
    ([&brokerage, &stats, &api_timer_stat]{
        statsd::timing_scope timer{stats, api_timer_stat};
        crow::json::wvalue x;
        const auto accounts = brokerage.accounts_under_management("GBP");
        for (const auto& [name, balance]: accounts) {
            x[name] = round(balance);
        }
        return CachelessJsonResponse{x};
    });

    CROW_ROUTE(app, "/account/<string>")
    ([&brokerage, &stats, &api_timer_stat](const auto& api_key){
        statsd::timing_scope timer{stats, api_timer_stat};
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
    ([&brokerage, &stats, &api_timer_stat](const auto& api_key,
                  const auto& direction,
                  const auto& ccy_pair,
                  double amount) {
        statsd::timing_scope timer{stats, api_timer_stat};
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

    std::thread api_app([&]{
        app.loglevel(crow::LogLevel::Warning);
        app.port(config["port"].i()).multithreaded().run();
    });

    //--------------------------------------------------------------------------
    // Runtime game configuration
    crow::SimpleApp config_app;

    CROW_ROUTE(config_app, "/")([&intervals](){
        crow::mustache::context x;
        x["trade_interval"] = intervals.trade_interval;
        x["chaos_interval"] = intervals.chaos_interval;
        x["arbitrage_interval"] = intervals.arbitrage_interval;
        x["saving_interval"] = intervals.saving_interval;
    
        auto page = crow::mustache::load("config.html");
        return HtmlResponse{page.render(x)};
    });

    CROW_ROUTE(config_app, "/update")
    .methods("POST"_method)
    ([&intervals](const crow::request& req){
        auto x = crow::json::load(req.body);
        if (!x) return crow::response(400);

        int trade_interval = x["trade_interval"].i();
        int chaos_interval = x["chaos_interval"].i();
        int arbitrage_interval = x["arbitrage_interval"].i();
        int saving_interval = x["saving_interval"].i();

        if (trade_interval < 0 || chaos_interval < 0
            || arbitrage_interval < 0 || saving_interval < 0)
        {
            return crow::response(400);
        }

        intervals.trade_interval = trade_interval;
        intervals.chaos_interval = chaos_interval;
        intervals.arbitrage_interval = arbitrage_interval;
        intervals.saving_interval = saving_interval;

        return crow::response(200);
    });

    config_app.port(8081).multithreaded().run();
}

