#pragma once

#include <string>
#include <cstdint>
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using json = nlohmann::json;

struct Config {
    uint16_t port = 18080;
    std::string host = "0.0.0.0";
    int session_timeout = 60;
    int reaper_interval = 10;
    std::string log_level = "info";

    static Config load(const std::string& filename = "config.json") {
        Config config;
        
        try {
            std::ifstream file(filename);
            if (!file.is_open()) {
                std::cout << "Config file not found, using defaults" << std::endl;
                return config;
            }

            json j;
            file >> j;
            
            if (j.contains("port")) config.port = j["port"];
            if (j.contains("host")) config.host = j["host"];
            if (j.contains("session_timeout")) config.session_timeout = j["session_timeout"];
            if (j.contains("reaper_interval")) config.reaper_interval = j["reaper_interval"];
            if (j.contains("log_level")) config.log_level = j["log_level"];

            std::cout << "Config loaded from " << filename << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "Error loading config: " << e.what() << std::endl;
            std::cout << "Using default configuration" << std::endl;
        }

        return config;
    }

    void print() const {
        std::cout << "\n=== Server Configuration ===" << std::endl;
        std::cout << "Host: " << host << std::endl;
        std::cout << "Port: " << port << std::endl;
        std::cout << "Session Timeout: " << session_timeout << "s" << std::endl;
        std::cout << "Reaper Interval: " << reaper_interval << "s" << std::endl;

        std::cout << "Log Level: " << log_level << std::endl;
        std::cout << "===========================\n" << std::endl;
    }
};
