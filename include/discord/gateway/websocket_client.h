#pragma once

#include <string>
#include <functional>
#include <nlohmann/json.hpp>
#include "reconnection.h"

namespace discord {

enum class GatewayOpcode {
    DISPATCH = 0,
    HEARTBEAT = 1,
    IDENTIFY = 2,
    PRESENCE_UPDATE = 3,
    VOICE_STATE_UPDATE = 4,
    RESUME = 6,
    RECONNECT = 7,
    REQUEST_GUILD_MEMBERS = 8,
    INVALID_SESSION = 9,
    HELLO = 10,
    HEARTBEAT_ACK = 11
};

enum class GatewayCloseEvent {
    UNKNOWN_ERROR = 4000,
    UNKNOWN_OPCODE = 4001,
    DECODE_ERROR = 4002,
    NOT_AUTHENTICATED = 4003,
    AUTHENTICATION_FAILED = 4004,
    ALREADY_AUTHENTICATED = 4005,
    INVALID_SEQ = 4007,
    RATE_LIMITED = 4008,
    SESSION_TIMED_OUT = 4009,
    INVALID_SHARD = 4010,
    SHARDING_REQUIRED = 4011,
    INVALID_API_VERSION = 4012,
    INVALID_INTENTS = 4013,
    DISALLOWED_INTENTS = 4014
};

class WebSocketClient {
public:
    using EventCallback = std::function<void(const nlohmann::json&)>;
    using CloseCallback = std::function<void(int, const std::string&)>;

    WebSocketClient();
    ~WebSocketClient();

    bool connect(const std::string& url);
    void disconnect();
    bool is_connected() const;

    void send(const nlohmann::json& payload);
    
    void on_event(EventCallback callback);
    void on_close(CloseCallback callback);

    void set_token(const std::string& token);
    void set_intents(int intents);
    void identify();
    
    // Reconnection management
    void enable_auto_reconnect(bool enabled);
    void set_reconnection_config(int max_retries = 5, 
                               std::chrono::milliseconds base_delay = std::chrono::milliseconds(1000),
                               std::chrono::milliseconds max_delay = std::chrono::milliseconds(30000));
    bool is_reconnecting() const;
    void stop_reconnecting();
    
    // Compression support
    void enable_compression(bool enabled);
    bool is_compression_enabled() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}