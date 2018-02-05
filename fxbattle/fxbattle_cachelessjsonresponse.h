#pragma once

#include <crow/crow_all.h>

namespace fxbattle {
struct CachelessJsonResponse : crow::response
{
    CachelessJsonResponse(const crow::json::wvalue& body)
            : crow::response {crow::json::dump(body)}
    {
        add_header("Cache-Control", "no-cache");
        add_header("Content-Type", "application/json");
    }
};
} // namespace fxbattle

