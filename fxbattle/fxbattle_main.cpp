#include <crow/crow_all.h>

int main()
{
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")([](){
        crow::mustache::context x;
    
        auto page = crow::mustache::load("index.html");
        return page.render(x);
    });

    app.port(18080).multithreaded().run();
}

