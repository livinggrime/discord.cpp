#include <discord/config.h>
#include <fstream>
#include <cstdlib>

namespace discord {

Config Config::from_file(const std::string& filename) {
    Config config;
    std::ifstream file(filename);
    if (file.is_open()) {
        try {
            nlohmann::json j;
            file >> j;
            config.from_json(j);
        } catch (const std::exception& e) {
            // Use default config if file is invalid
        }
    }
    return config;
}

Config Config::from_env() {
    Config config;
    
    const char* token = std::getenv("DISCORD_BOT_TOKEN");
    if (token) {
        config.token = token;
    }
    
    const char* api_version = std::getenv("DISCORD_API_VERSION");
    if (api_version) {
        config.api_version = api_version;
    }
    
    const char* base_url = std::getenv("DISCORD_BASE_URL");
    if (base_url) {
        config.base_url = base_url;
    }
    
    const char* gateway_url = std::getenv("DISCORD_GATEWAY_URL");
    if (gateway_url) {
        config.gateway_url = gateway_url;
    }
    
    return config;
}

nlohmann::json Config::to_json() const {
    return nlohmann::json{
        {"token", token},
        {"api_version", api_version},
        {"base_url", base_url},
        {"gateway_url", gateway_url},
        {"intents", intents},
        {"compress", compress},
        {"large_threshold", large_threshold},
        {"heartbeat_interval", heartbeat_interval.count()},
        {"connection_timeout", connection_timeout.count()},
        {"request_timeout", request_timeout.count()},
        {"max_retries", max_retries},
        {"retry_delay", retry_delay.count()},
        {"user_agent", user_agent},
        {"rate_limit", nlohmann::json{
            {"max_requests", rate_limit.max_requests},
            {"window", rate_limit.window.count()}
        }},
        {"sharding", nlohmann::json{
            {"shard_id", sharding.shard_id},
            {"shard_count", sharding.shard_count}
        }}
    };
}

void Config::from_json(const nlohmann::json& j) {
    if (j.contains("token")) j.at("token").get_to(token);
    if (j.contains("api_version")) j.at("api_version").get_to(api_version);
    if (j.contains("base_url")) j.at("base_url").get_to(base_url);
    if (j.contains("gateway_url")) j.at("gateway_url").get_to(gateway_url);
    if (j.contains("intents")) j.at("intents").get_to(intents);
    if (j.contains("compress")) j.at("compress").get_to(compress);
    if (j.contains("large_threshold")) j.at("large_threshold").get_to(large_threshold);
    if (j.contains("heartbeat_interval")) heartbeat_interval = std::chrono::milliseconds(j["heartbeat_interval"]);
    if (j.contains("connection_timeout")) connection_timeout = std::chrono::milliseconds(j["connection_timeout"]);
    if (j.contains("request_timeout")) request_timeout = std::chrono::milliseconds(j["request_timeout"]);
    if (j.contains("max_retries")) j.at("max_retries").get_to(max_retries);
    if (j.contains("retry_delay")) retry_delay = std::chrono::milliseconds(j["retry_delay"]);
    if (j.contains("user_agent")) j.at("user_agent").get_to(user_agent);
    
    if (j.contains("rate_limit")) {
        const auto& rl = j["rate_limit"];
        if (rl.contains("max_requests")) rl.at("max_requests").get_to(rate_limit.max_requests);
        if (rl.contains("window")) rate_limit.window = std::chrono::milliseconds(rl["window"]);
    }
    
    if (j.contains("sharding")) {
        const auto& s = j["sharding"];
        if (s.contains("shard_id")) s.at("shard_id").get_to(sharding.shard_id);
        if (s.contains("shard_count")) s.at("shard_count").get_to(sharding.shard_count);
    }
}

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::load_config(const Config& config) {
    config_ = config;
}

const Config& ConfigManager::get_config() const {
    return config_;
}

void ConfigManager::set_token(const std::string& token) {
    config_.token = token;
}

void ConfigManager::set_intents(int intents) {
    config_.intents = intents;
}

void ConfigManager::set_sharding(int shard_id, int shard_count) {
    config_.sharding.shard_id = shard_id;
    config_.sharding.shard_count = shard_count;
}

}