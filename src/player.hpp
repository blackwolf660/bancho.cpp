#pragma once

#include <string>
#include <cstdint>

#include <random>
#include <sstream>
#include <iomanip>

#include <atomic>

#include <chrono>

enum class Action : uint8_t {
    Idle = 0,
    Afk = 1,
    Playing = 2,
    Editing = 3,
    Modding = 4,
    Multiplayer = 5,
    Watching = 6,
    Unknown = 7,
    Testing = 8,
    Submitting = 9,
    Paused = 10,
    Lobby = 11,
    Multiplaying = 12,
    OsuDirect = 13
};

struct PlayerStatus {
    Action action = Action::Idle;
    std::string info_text = "";
    std::string map_md5 = "";
    int32_t mods = 0;
    uint8_t mode_as_vanilla = 0;
    int32_t map_id = 0;
};

struct ModeData {
    int64_t tscore = 0;
    int64_t rscore = 0;
    int32_t pp = 0;
    float acc = 0.0f;
    int32_t plays = 0;
    int32_t playtime = 0;
    int32_t max_combo = 0;
    int32_t total_hits = 0;
    int32_t rank = 0;
};

class Player {
public:
    int32_t id = 0;
    std::string name = "";
    std::string name_safe = "";
    std::string token = "";
    
    uint32_t priv = 1 << 2 | 1;
    int8_t utc_offset = 0;

    float longitude = 0.0f;
    float latitude = 0.0f;
    uint8_t country_code = 0;

    PlayerStatus status;
    ModeData stats;

    bool pm_private = false;
    int32_t silence_end = 0;

    std::vector<uint8_t> queue;

    // --- Session Storage ---
    std::chrono::steady_clock::time_point login_time;
    std::atomic<std::chrono::steady_clock::time_point> last_recv_time;
    // Main map
    inline static std::unordered_map<int32_t, std::shared_ptr<Player>> by_id;
    // Addition map for search
    inline static std::unordered_map<std::string, std::shared_ptr<Player>> by_token;
    inline static std::unordered_map<std::string, std::shared_ptr<Player>> by_name;

    inline static std::atomic<int32_t> next_id{3};

    Player() {}
    Player(int32_t id, std::string name) : id(id), name(name) {
        this->name_safe = name;
        std::transform(name_safe.begin(), name_safe.end(), name_safe.begin(), ::tolower);
        this->login_time = std::chrono::steady_clock::now();
        this->last_recv_time = std::chrono::steady_clock::now();
    }

    static void add_to_sessions(std::shared_ptr<Player> p) {
        by_id[p->id] = p;
        if (!p->name_safe.empty()) by_name[p->name_safe] = p;
        if (!p->token.empty()) by_token[p->token] = p;
    }

    static void remove_from_sessions(int32_t id) {
        auto it = by_id.find(id);
        if (it != by_id.end()) {
            by_token.erase(it->second->token);
            by_name.erase(it->second->name_safe);
            by_id.erase(it);
        }
    }

    Action action() const { return status.action; }
    const std::string& info_text() const { return status.info_text; }
    const std::string& map_md5() const { return status.map_md5; }
    int32_t mods() const { return status.mods; }
    int32_t map_id() const { return status.map_id; }
    int32_t rank() const { return stats.rank; }
    float accuracy() const { return stats.acc; }

    static std::shared_ptr<Player> get_by_token(const std::string& t) {
        auto it = by_token.find(t);
        return (it != by_token.end()) ? it->second : nullptr;
    }

    static std::shared_ptr<Player> get_by_id(int32_t id) {
        auto it = by_id.find(id);
        return (it != by_id.end()) ? it->second : nullptr;
    }

    static std::shared_ptr<Player> get_by_name(std::string name) {
        std::transform(name.begin(), name.end(), name.begin(), ::tolower);
        auto it = by_name.find(name);
        return (it != by_name.end()) ? it->second : nullptr;
    }

    void enqueue(const std::vector<uint8_t>& packet) {
        queue.insert(queue.end(), packet.begin(), packet.end());
    }

    std::optional<std::vector<uint8_t>> dequeue() {
        if (queue.empty()) {
            return std::nullopt;
        }

        std::vector<uint8_t> data = std::move(queue);
        
        queue.clear(); 
        
        return data;
    }

    static std::string generate_token() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static const char* digits = "0123456789abcdef";

        std::stringstream ss;
        for (int i = 0; i < 32; ++i) {
            if (i == 8 || i == 12 || i == 16 || i == 20) {
                ss << "-";
            }
            ss << digits[dis(gen)];
        }
        return ss.str();
    }

    static int32_t get_next_id() {
        return next_id.fetch_add(1);
    }

    void update_activity() {
        last_recv_time = std::chrono::steady_clock::now();
    }

    void logout();
};