#pragma once

#include <crow/crow_all.h>

namespace fxbattle {
struct HtmlResponse : crow::response
{
    HtmlResponse(const std::string& body)
            : crow::response {body}
    {
        add_header("Content-Type", "text/html");
    }
};
} // namespace fxbattle

