#pragma once

#include <crow/crow_all.h>

#include <exchange_market.h>
#include <exchange_brokerage.h>

namespace fxbattle {

crow::json::rvalue get_config(std::string filename);

exchange::Market get_market(std::string filename);

exchange::Brokerage get_brokerage(
        std::string filename,
        const crow::json::rvalue& config,
        exchange::Market& market);

} // namespace fxbattle

