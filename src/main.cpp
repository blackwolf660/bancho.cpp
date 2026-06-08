#include <iostream>
#include <thread>
#include <chrono>

#include "cho.hpp"
#include "config.hpp"

void session_reaper(const Config& config) {
    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(config.reaper_interval));

        auto now = std::chrono::steady_clock::now();
        std::vector<std::shared_ptr<Player>> to_disconnect;

        for (auto const& [id, p] : Player::by_id) {
            auto last = p->last_recv_time.load();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - last);

            if (duration.count() > config.session_timeout) {
                to_disconnect.push_back(p);
            }
        }

        for (auto& p : to_disconnect) {
            std::cout << "Player " << p->name << " timed out." << std::endl;
            p->logout();
        }
    }
}

int main() {
    // Load configuration
    Config config = Config::load();
    config.print();
    
    std::thread(session_reaper, config).detach();
    
    crow::SimpleApp app;

    CROW_ROUTE(app, "/")
        .methods("POST"_method)
    ([](const crow::request& request){
        return bancho_handler(request);
    });

    app.port(config.port).multithreaded().run();

    return 0;
}