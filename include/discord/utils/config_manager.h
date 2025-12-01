#pragma once

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

namespace discord {

struct Config {
    std::string token;
    std::string api_version = "10";
    std::string base_url = "https://discord.com/api";
    std::string gateway_url = "wss://gateway.discord.gg";
    
    int intents = 0;
    bool compress = false;
    bool large_threshold = 50;
    
    std::chrono::milliseconds heartbeat_interval{42500};
    std::chrono::milliseconds connection_timeout{5000};
    std::chrono::milliseconds request_timeout{30000};
    
    int max_retries = 3;
    std::chrono::milliseconds retry_delay{1000};
    
    std::string user_agent = "DiscordBot (https://github.com/yourusername/discord.cpp, 1.0.0)";
    
    struct RateLimit {
        int max_requests = 5;
        std::chrono::milliseconds window{5000};
    } rate_limit;
    
    struct Sharding {
        int shard_id = 0;
        int shard_count = 1;
    } sharding;
    
    static Config from_file(const std::string& filename);
    static Config from_env();
    nlohmann::json to_json() const;
    void from_json(const nlohmann::json& j);
};

class ConfigManager {
public:
    static ConfigManager& instance();
    
    void load_config(const Config& config);
    const Config& get_config() const;
    
    void set_token(const std::string& token);
    void set_intents(int intents);
    void set_sharding(int shard_id, int shard_count);

private:
    ConfigManager() = default;
    Config config_;
};

}