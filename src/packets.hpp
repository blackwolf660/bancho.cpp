#pragma once

#include <string>
#include <cstdint>

#include <unordered_map>
#include <functional>
#include <memory>
#include <vector>
#include <span>
#include <bit>

#include "player.hpp"

enum class ClientPackets : uint16_t {
    CHANGE_ACTION = 0,
    SEND_PUBLIC_MESSAGE = 1,
    LOGOUT = 2,
    REQUEST_STATUS_UPDATE = 3,
    PING = 4,
    START_SPECTATING = 16,
    STOP_SPECTATING = 17,
    SPECTATE_FRAMES = 18,
    ERROR_REPORT = 20,
    CANT_SPECTATE = 21,
    SEND_PRIVATE_MESSAGE = 25,
    PART_LOBBY = 29,
    JOIN_LOBBY = 30,
    CREATE_MATCH = 31,
    JOIN_MATCH = 32,
    PART_MATCH = 33,
    MATCH_CHANGE_SLOT = 38,
    MATCH_READY = 39,
    MATCH_LOCK = 40,
    MATCH_CHANGE_SETTINGS = 41,
    MATCH_START = 44,
    MATCH_SCORE_UPDATE = 47,
    MATCH_COMPLETE = 49,
    MATCH_CHANGE_MODS = 51,
    MATCH_LOAD_COMPLETE = 52,
    MATCH_NO_BEATMAP = 54,
    MATCH_NOT_READY = 55,
    MATCH_FAILED = 56,
    MATCH_HAS_BEATMAP = 59,
    MATCH_SKIP_REQUEST = 60,
    CHANNEL_JOIN = 63,
    BEATMAP_INFO_REQUEST = 68,
    MATCH_TRANSFER_HOST = 70,
    FRIEND_ADD = 73,
    FRIEND_REMOVE = 74,
    MATCH_CHANGE_TEAM = 77,
    CHANNEL_PART = 78,
    RECEIVE_UPDATES = 79,
    SET_AWAY_MESSAGE = 82,
    IRC_ONLY = 84,
    USER_STATS_REQUEST = 85,
    MATCH_INVITE = 87,
    MATCH_CHANGE_PASSWORD = 90,
    TOURNAMENT_MATCH_INFO_REQUEST = 93,
    USER_PRESENCE_REQUEST = 97,
    USER_PRESENCE_REQUEST_ALL = 98,
    TOGGLE_BLOCK_NON_FRIEND_DMS = 99,
    TOURNAMENT_JOIN_MATCH_CHANNEL = 108,
    TOURNAMENT_LEAVE_MATCH_CHANNEL = 109
};

enum class ServerPackets : uint16_t {
    USER_ID = 5,
    SEND_MESSAGE = 7,
    PONG = 8,
    HANDLE_IRC_QUIT = 10,
    USER_STATS = 11,
    USER_LOGOUT = 12,
    SPECTATOR_JOINED = 13,
    SPECTATOR_LEFT = 14,
    SPECTATE_FRAMES = 15,
    VERSION_UPDATE = 19,
    SPECTATOR_CANT_SPECTATE = 22,
    GET_ATTENTION = 23,
    NOTIFICATION = 24,
    UPDATE_MATCH = 26,
    NEW_MATCH = 27,
    DISPOSE_MATCH = 28,
    TOGGLE_BLOCK_NON_FRIEND_DMS = 34,
    MATCH_JOIN_SUCCESS = 36,
    MATCH_JOIN_FAIL = 37,
    FELLOW_SPECTATOR_JOINED = 42,
    FELLOW_SPECTATOR_LEFT = 43,
    ALL_PLAYERS_LOADED = 45,
    MATCH_START = 46,
    MATCH_SCORE_UPDATE = 48,
    MATCH_TRANSFER_HOST = 50,
    MATCH_ALL_PLAYERS_LOADED = 53,
    MATCH_PLAYER_FAILED = 57,
    MATCH_COMPLETE = 58,
    MATCH_SKIP = 61,
    CHANNEL_JOIN_SUCCESS = 64,
    CHANNEL_INFO = 65,
    CHANNEL_KICK = 66,
    CHANNEL_AUTO_JOIN = 67,
    BEATMAP_INFO_REPLY = 69,
    PRIVILEGES = 71,
    FRIENDS_LIST = 72,
    PROTOCOL_VERSION = 75,
    MAIN_MENU_ICON = 76,
    MONITOR = 80,
    MATCH_PLAYER_SKIPPED = 81,
    USER_PRESENCE = 83,
    RESTART = 86,
    MATCH_INVITE = 88,
    CHANNEL_INFO_END = 89,
    MATCH_CHANGE_PASSWORD = 91,
    SILENCE_END = 92,
    USER_SILENCED = 94,
    USER_PRESENCE_SINGLE = 95,
    USER_PRESENCE_BUNDLE = 96,
    USER_DM_BLOCKED = 100,
    TARGET_IS_SILENCED = 101,
    VERSION_UPDATE_FORCED = 102,
    SWITCH_SERVER = 103,
    ACCOUNT_RESTRICTED = 104,
    MATCH_ABORT = 106,
    SWITCH_TOURNAMENT_SERVER = 107
};

enum class OsuTypes : uint8_t {
    i8 = 0, u8 = 1, i16 = 2, u16 = 3,
    i32 = 4, u32 = 5, f32 = 6, i64 = 7,
    u64 = 8, f64 = 9,

    message = 11,
    channel = 12,
    match = 13,
    scoreframe = 14,
    mapInfoRequest = 15,
    mapInfoReply = 16,
    replayFrameBundle = 17,

    i32_list = 18,
    i32_list4l = 19,
    string = 20,
    raw = 21
};

class PacketWriter {
public:
    std::vector<uint8_t> buffer;

    PacketWriter(uint16_t packetId) {
        write_u16(packetId);
        write_u8(0);
    }

    void write_u8(uint8_t v)  { buffer.push_back(v); }
    void write_i16(int16_t v) { write_raw(&v, 2); }
    void write_u16(uint16_t v){ write_raw(&v, 2); }
    void write_i32(int32_t v) { write_raw(&v, 4); }
    void write_u32(uint32_t v){ write_raw(&v, 4); }
    void write_i64(int64_t v) { write_raw(&v, 8); }
    void write_f32(float v)   { write_raw(&v, 4); }

    void write_string(const std::string& str) {
        if (str.empty()) {
            write_u8(0);
            return;
        }
        write_u8(11);
        write_uleb128(str.size());
        write_raw(str.data(), str.size());
    }

    void write_raw(const void* data, size_t len) {
        const uint8_t* p = reinterpret_cast<const uint8_t*>(data);
        buffer.insert(buffer.end(), p, p + len);
    }

    std::vector<uint8_t> finalize() {
        uint32_t size = static_cast<uint32_t>(buffer.size() - 3);

        uint8_t size_bytes[4];
        std::memcpy(size_bytes, &size, 4);
        buffer.insert(buffer.begin() + 3, size_bytes, size_bytes + 4);
        return std::move(buffer);
    }

    void write_i32_list(const std::vector<int32_t>& list) {
        write_u16(static_cast<uint16_t>(list.size()));
        for (int32_t item : list) write_i32(item);
    }

    void write_osu_message(const std::string& sender, const std::string& msg, const std::string& target, int32_t sender_id) {
        write_string(sender);
        write_string(msg);
        write_string(target);
        write_i32(sender_id);
    }

    void write_osu_channel(const std::string& name, const std::string& topic, int32_t count) {
        write_string(name);
        write_string(topic);
        write_i32(count);
    }

private:
    void write_uleb128(size_t value) {
        while (value >= 0x80) {
            buffer.push_back((uint8_t)((value & 0x7F) | 0x80));
            value >>= 7;
        }
        buffer.push_back((uint8_t)value);
    }
};
using Packet = std::vector<uint8_t>;

Packet protocol_version(int32_t ver) {
    PacketWriter p((uint16_t)ServerPackets::PROTOCOL_VERSION);
    p.write_i32(ver);
    return p.finalize();
}

Packet user_id(int32_t id) {
    PacketWriter p((uint16_t)ServerPackets::USER_ID);
    p.write_i32(id);
    return p.finalize();
}

Packet notification(const std::string& msg) {
    PacketWriter p((uint16_t)ServerPackets::NOTIFICATION);
    p.write_string(msg);
    return p.finalize();
}

Packet channel_info(const std::string& name, const std::string& topic, int32_t count) {
    PacketWriter p((uint16_t)ServerPackets::CHANNEL_INFO);
    p.write_osu_channel(name, topic, count);
    return p.finalize();
}

Packet channel_info_end() {
    return PacketWriter((uint16_t)ServerPackets::CHANNEL_INFO_END).finalize();
}

Packet friends_list(const std::vector<int32_t>& friends) {
    PacketWriter p((uint16_t)ServerPackets::FRIENDS_LIST);
    p.write_i32_list(friends);
    return p.finalize();
}

Packet main_menu_icon(const std::string& icon_url, const std::string& onclick_url) {
    PacketWriter p((uint16_t)ServerPackets::MAIN_MENU_ICON);
    p.write_string(icon_url + "|" + onclick_url);
    return p.finalize();
}

Packet send_message(const std::string& sender, const std::string& msg, const std::string& target, int32_t sender_id) {
    PacketWriter p((uint16_t)ServerPackets::SEND_MESSAGE);
    p.write_osu_message(sender, msg, target, sender_id);
    return p.finalize();
}

Packet user_presence(const Player& player) {
    PacketWriter p((uint16_t)ServerPackets::USER_PRESENCE);
    
    p.write_i32(player.id);
    p.write_string(player.name);
    
    p.write_u8(static_cast<uint8_t>(player.utc_offset + 24));
    
    p.write_u8(player.country_code); 
    
    uint8_t privs = static_cast<uint8_t>(player.priv);
    uint8_t mode = player.status.mode_as_vanilla;
    
    uint8_t combined_byte = privs | (mode << 5);
    p.write_u8(combined_byte);
    
    p.write_f32(player.longitude);
    p.write_f32(player.latitude);
    
    p.write_i32(player.stats.rank); 
    
    return p.finalize();
}

Packet user_stats(const Player& p) {
    PacketWriter pw((uint16_t)ServerPackets::USER_STATS);
    
    int64_t rscore = (p.stats.pp > 0xFFFF) ? (int64_t)p.stats.pp : p.stats.rscore;
    uint16_t pp_display = (p.stats.pp > 0xFFFF) ? 0 : static_cast<uint16_t>(p.stats.pp);

    pw.write_i32(p.id);
    
    pw.write_u8(static_cast<uint8_t>(p.action())); 
    pw.write_string(p.info_text());
    pw.write_string(p.map_md5());
    pw.write_i32(p.mods());
    
    pw.write_u8(p.status.mode_as_vanilla);
    pw.write_i32(p.map_id());
    
    pw.write_i64(rscore);
    pw.write_f32(p.stats.acc / 100.0f);
    pw.write_i32(p.stats.plays);
    pw.write_i64(p.stats.tscore);
    pw.write_i32(p.stats.rank);
    
    pw.write_u16(pp_display);
    
    return pw.finalize();
}

Packet privileges(int32_t priv) {
    PacketWriter p((uint16_t)ServerPackets::PRIVILEGES);
    p.write_i32(priv);
    return p.finalize();
}

Packet silence_end(int32_t delta) {
    PacketWriter p((uint16_t)ServerPackets::SILENCE_END);
    p.write_i32(delta);
    return p.finalize();
}

Packet ping() {
    PacketWriter p((uint16_t)ServerPackets::PONG);
    return p.finalize();
}

Packet user_presence_single(int32_t id) {
    PacketWriter p((uint16_t)ServerPackets::USER_PRESENCE_SINGLE);
    p.write_i32(id);
    return p.finalize();
}

Packet user_presence_bundle(const std::vector<int32_t>& ids) {
    PacketWriter p((uint16_t)ServerPackets::USER_PRESENCE_BUNDLE);
    p.write_i32_list(ids); 
    return p.finalize();
}

Packet user_logout(int32_t id) {
    PacketWriter p((uint16_t)ServerPackets::USER_LOGOUT);
    p.write_i32(id);
    return p.finalize();
}

Packet channel_join(std::string& name) {
    PacketWriter p((uint16_t)ServerPackets::CHANNEL_JOIN_SUCCESS);
    p.write_string(name);
    return p.finalize();
}

inline void Player::logout() {
    std::vector<uint8_t> logout_packet = user_logout(this->id);
    
    for (auto const& [other_id, other_ptr] : by_id) {
        if (other_id != this->id) {
            other_ptr->enqueue(logout_packet);
        }
    }

    Player::remove_from_sessions(this->id);
}

struct Message {
    std::string sender;
    std::string text;
    std::string recipient;
    int32_t sender_id;
};

class Channel {
public:
    std::string name;
    std::string topic;
    // std::string read_priv;
    // std::string write_priv;
    bool auto_join;
    bool instance;
    Channel() {}
    Channel(std::string name, std::string topic) {
        this->name = name;
        this->topic = topic;
    }
    void send(Message msg, Player& p, bool to_self = false) {
        for (auto const& [id, other] : Player::by_id) {
            if (other->id == p.id || !to_self) continue;

            other->enqueue(send_message(
                p.name,
                msg.text,
                this->name,
                p.id
            ));
        }
    }
};

class BasePacket;
class BanchoPacketReader;

using PacketFactory = std::function<std::unique_ptr<BasePacket>(BanchoPacketReader&)>;
using PacketMap = std::unordered_map<ClientPackets, PacketFactory>;

class BanchoPacketReader {
private:
    std::span<const uint8_t> data;
    size_t offset = 0;
    uint32_t current_packet_len = 0;

public:
    BanchoPacketReader(std::span<const uint8_t> body) : data(body) {}

    bool empty() const { return offset >= data.size(); }

    std::pair<ClientPackets, uint32_t> read_header() {
        auto p_type = static_cast<ClientPackets>(read_u16());
        offset += 1;
        current_packet_len = read_u32();
        return {p_type, current_packet_len};
    }

    std::span<const uint8_t> read_raw(size_t len) {
        auto sub = data.subspan(offset, len);
        offset += len;
        return sub;
    }

    template<typename T>
    T read_t() {
        T val;
        std::memcpy(&val, &data[offset], sizeof(T));

        offset += sizeof(T);
        return val;
    }

    int8_t  read_i8()  { return read_t<int8_t>(); }
    uint8_t read_u8()  { return read_t<uint8_t>(); }
    int16_t read_i16() { return read_t<int16_t>(); }
    uint16_t read_u16() { return read_t<uint16_t>(); }
    int32_t read_i32() { return read_t<int32_t>(); }
    uint32_t read_u32() { return read_t<uint32_t>(); }
    float   read_f32() { return read_t<float>(); }
    double  read_f64() { return read_t<double>(); }

    std::string read_string() {
        uint8_t indicator = read_u8();
        if (indicator != 0x0B) return "";

        size_t length = 0;
        int shift = 0;
        while (true) {
            uint8_t byte = read_u8();
            length |= (static_cast<size_t>(byte & 0x7F) << shift);
            if ((byte & 0x80) == 0) break;
            shift += 7;
        }

        if (length == 0) return "";
        
        auto raw = read_raw(length);
        return std::string(reinterpret_cast<const char*>(raw.data()), raw.size());
    }

    std::unique_ptr<BasePacket> next_packet(const PacketMap& packet_map) {
        while (offset + 7 <= data.size()) {
            auto [p_type, p_len] = read_header();

            if (!packet_map.contains(p_type)) {
                offset += p_len;
                continue;
            }

            return packet_map.at(p_type)(*this);
        }
        return nullptr;
    }

    Message read_message() {
        return Message{
            .sender = read_string(),
            .text = read_string(),
            .recipient = read_string(),
            .sender_id = read_i32()
        };
    }
};

class BasePacket {
public:
    BasePacket(BanchoPacketReader& reader) {}

    virtual ~BasePacket() = default;
    virtual void handle(Player& p) = 0;
};

struct PacketStorage {
    inline static std::unordered_map<ClientPackets, PacketFactory> all;
    inline static std::unordered_map<ClientPackets, PacketFactory> restricted;
};

template <typename T>
struct PacketRegister {
    PacketRegister(ClientPackets id, bool restricted = false) {
        auto factory = [](BanchoPacketReader& r) { 
            return std::make_unique<T>(r); 
        };

        PacketStorage::all[id] = factory;
        
        if (restricted) {
            PacketStorage::restricted[id] = factory;
        }
    }
};

#define REGISTER_PACKET(Class, ID, Restricted) inline static PacketRegister<Class> _reg_##Class{ID, Restricted};

class Ping : public BasePacket {
public:
    Ping(BanchoPacketReader& reader) : BasePacket(reader) {}

    void handle(Player& p) override {
        std::cout << "Ping handled!" << std::endl;
    }

private:
    REGISTER_PACKET(Ping, ClientPackets::PING, true)
};

class SendPrivateMessage : public BasePacket {
    Message message;
public:
    SendPrivateMessage(BanchoPacketReader& reader) : BasePacket(reader) {
        message = reader.read_message();
    }

    void handle(Player& p) override {
        std::cout << "SendPrivateMessage handled! "
            << message.recipient << "|"
            << message.sender << "|"
            << p.name << "|"
            << message.sender_id << "|"
            << message.text << "|"
            << std::endl;

        std::shared_ptr<Player> target = Player::get_by_name(message.recipient);
        if(target) {
            target->enqueue(send_message(p.name, message.text, message.recipient, message.sender_id));
        } else {
            p.enqueue(send_message(message.recipient, "Player not found!", message.recipient, message.sender_id));
        }
    }

private:
    REGISTER_PACKET(SendPrivateMessage, ClientPackets::SEND_PRIVATE_MESSAGE, true)
};

class Logout : public BasePacket {
public:
    Logout(BanchoPacketReader& reader) : BasePacket(reader) {
        reader.read_i32();
    }

    void handle(Player& p) override {
        auto now = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - p.login_time);

        if (duration.count() < 1) {
            std::cout << "Logout ignored (too fast): " << p.name << std::endl;
            return;
        }

        std::cout << "Player logging out: " << p.name << std::endl;
        p.logout();
    }

private:
    REGISTER_PACKET(Logout, ClientPackets::LOGOUT, true)
};

class SendPublicMessage : public BasePacket {
    Message message;
public:
    SendPublicMessage(BanchoPacketReader& reader) : BasePacket(reader) {
        message = reader.read_message();
    }

    void handle(Player& p) override {
        std::cout << "SendPublicMessage handled! "
            << message.recipient << "|"
            << message.sender << "|"
            << p.name << "|"
            << message.sender_id << "|"
            << message.text << "|"
            << std::endl;

        Channel chan{"#osu", "Huila chat"};
        chan.send(message, p);
    }
private:
    REGISTER_PACKET(SendPublicMessage, ClientPackets::SEND_PUBLIC_MESSAGE, true)
};

class ChannelJoin : public BasePacket {
    std::string name;
public:
    ChannelJoin(BanchoPacketReader& reader) : BasePacket(reader) {
        name = reader.read_string();
    }

    void handle(Player& p) override {
        std::cout << "ChannelJoin handled!" << '\n';

        Channel chan{"#osu", "Huila chat"};
        
        p.enqueue(channel_join(chan.name));
    }
private:
    REGISTER_PACKET(ChannelJoin, ClientPackets::CHANNEL_JOIN, true)
};