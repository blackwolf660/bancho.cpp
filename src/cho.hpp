#pragma once

#include <string>
#include <cstdint>
#include <vector>
#include <optional>
#include <span>

#include "crow.h"

#include "packets.hpp"
#include "player.hpp"

void operator+=(std::vector<uint8_t>& dest, const std::vector<uint8_t>& src) {
    dest.insert(dest.end(), src.begin(), src.end());
}

struct LoginResponse {
    std::string osu_token;
    std::vector<uint8_t> response_body;
};

struct LoginData {
    std::string username;
    std::string password_md5;
    std::string osu_version;
    int utc_offset;
    bool display_city;
    bool pm_private;
    std::string osu_path_md5;
    std::string adapters_str;
    std::string adapters_md5;
    std::string uninstall_md5;
    std::string disk_signature_md5;
};

std::string_view shift_token(std::string_view& data, char delimiter) {
    size_t pos = data.find(delimiter);
    if (pos == std::string_view::npos) {
        std::string_view token = data;
        data = {}; 
        return token;
    }
    std::string_view token = data.substr(0, pos);
    data.remove_prefix(pos + 1);
    return token;
}

std::optional<LoginData> parse_login_data(const char* data, size_t length) {
    std::string_view body(data, length);

    try {
        std::string username(shift_token(body, '\n'));
        std::string password_md5(shift_token(body, '\n'));
        
        std::string osu_version(shift_token(body, '|'));
        int utc_offset = std::stoi(std::string(shift_token(body, '|')));
        bool display_city = (shift_token(body, '|') == "1");
        
        std::string_view client_hashes = shift_token(body, '|');
        bool pm_private = (shift_token(body, '|') == "1");

        if (!client_hashes.empty() && client_hashes.back() == ':') {
            client_hashes.remove_suffix(1);
        }

        std::string_view h_path = shift_token(client_hashes, ':');
        std::string_view h_adapters_str = shift_token(client_hashes, ':');
        std::string_view h_adapters_md5 = shift_token(client_hashes, ':');
        std::string_view h_uninstall = shift_token(client_hashes, ':');
        std::string_view h_disk = client_hashes; // остаток

        return LoginData{
            username,
            password_md5,
            osu_version,
            utc_offset,
            display_city,
            pm_private,
            std::string(h_path),
            std::string(h_adapters_str),
            std::string(h_adapters_md5),
            std::string(h_uninstall),
            std::string(h_disk)
        };

    } catch (...) {
        return std::nullopt;
    }
}

crow::response login(const LoginData& ld) {
    auto p = std::make_shared<Player>(Player::get_next_id(), ld.username);

    p->token = Player::generate_token();
    Player::add_to_sessions(p);
    p->utc_offset = ld.utc_offset;
    
    std::vector<uint8_t> data;
    
    data += protocol_version(19);
    data += user_id(p->id);

    if (ld.username == "Peppy" || ld.username == "peppy") {
        data += privileges(128);
        data += ping();
    }
    data += privileges(5);

    
    std::vector<uint8_t> new_player_data;
    new_player_data += user_presence(*p);
    new_player_data += user_stats(*p);

    std::vector<int> friends;
    friends.emplace_back(1);
    data += friends_list(friends);

    Channel chan{"#osu", "Just chat"};
    data += channel_info(chan.name, chan.topic, Player::next_id);
    data += channel_info_end();

    //////////////////////////////////////////////
    data += new_player_data;

    // --- ЦИКЛ ОПОВЕЩЕНИЯ ---
    for (auto const& [id, other] : Player::by_id) {
        if (other->id == p->id) continue;

        other->enqueue(new_player_data);

        data += user_presence(*other);
        data += user_stats(*other);
    }

    Player bot(1, "BanchoBot"); 
    data += user_presence(bot);
    data += user_stats(bot);

    std::vector<int32_t> all_ids;
    for (const auto& [id, _] : Player::by_id) all_ids.push_back(id);
    all_ids.push_back(1);
    data += user_presence_bundle(all_ids);
    //////////////////////////////////////////////

    data += main_menu_icon("https://example.com/","https://example.com/");

    data += silence_end(0);

    data += notification("Welcome to Bancho C++!");

    std::string body_str(data.begin(), data.end());
    crow::response res(body_str);

    res.set_header("cho-token", p->token);
    res.set_header("x-mcosu-features", "submit=1");
    res.set_header("Content-Type", "text/html; charset=UTF-8");

    return res;
}


void process_request(std::span<const uint8_t> body, Player& player) {
    BanchoPacketReader reader(body);

    while (!reader.empty()) {
        auto packet = reader.next_packet(PacketStorage::all);
        if (packet) {
            packet->handle(player);
        } else {
            break; 
        }
    }
}

crow::response bancho_handler(const crow::request& request) {    
    const std::string &osu_token = request.get_header_value("osu-token");
    const std::string &user_agent = request.get_header_value("User-Agent");

    if (osu_token.empty()) {
        auto ld = parse_login_data(request.body.data(), request.body.size());
        if (!ld) return crow::response(400);
        return login(*ld);
    }

    std::shared_ptr<Player> player = Player::get_by_token(osu_token);

    if (!player) {
        std::cout << "Player not found for token: " << osu_token << std::endl;
        return crow::response(403); 
    }

    player->update_activity();

    std::span<const uint8_t> body_view(
        reinterpret_cast<const uint8_t*>(request.body.data()), 
        request.body.size()
    );
    
    process_request(body_view, *player);

    auto pending_data = player->dequeue();
    if (pending_data) {
        std::string res_body(pending_data->begin(), pending_data->end());
        return crow::response(res_body);
    }

    return crow::response("");
}
