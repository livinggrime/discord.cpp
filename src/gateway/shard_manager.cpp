#include <discord/gateway/shard_manager.h>
#include <discord/utils/logger.h>
#include <discord/api/rest_endpoints.h>
#include <discord/core/exceptions.h>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

namespace discord {

ShardManager::ShardManager(const std::string& token, const ShardConfig& config)
    : config_(config), bot_token_(token) {
    
    if (!validate_config()) {
        throw DiscordException("Invalid shard configuration");
    }
    
    // Get gateway information
    session_info_ = get_gateway_info();
    
    if (config_.auto_sharding) {
        config_.shard_count = calculate_shard_count();
    }
    
    LOG_INFO("ShardManager initialized with " + std::to_string(config_.shard_count) + " shards");
}

ShardManager::~ShardManager() {
    stop();
}

bool ShardManager::start() {
    if (is_running_.load()) {
        LOG_WARN("ShardManager is already running");
        return false;
    }
    
    if (!validate_config()) {
        LOG_ERROR("Invalid shard configuration");
        return false;
    }
    
    is_running_ = true;
    is_shutting_down_ = false;
    
    LOG_INFO("Starting " + std::to_string(config_.shard_count) + " shards");
    
    // Start connection threads with concurrency limits
    for (int i = 0; i < config_.shard_count; ++i) {
        if (!can_start_session()) {
            wait_for_session_slot();
        }
        
        connection_threads_.emplace_back([this, i]() {
            if (is_shutting_down_.load()) return;
            
            try {
                connect_shard(i);
            } catch (const std::exception& e) {
                LOG_ERROR("Failed to connect shard " + std::to_string(i) + ": " + e.what());
            }
        });
        
        // Add delay between connections to respect rate limits
        if (i < config_.shard_count - 1) {
            std::this_thread::sleep_for(config_.connection_delay);
        }
    }
    
    // Wait for all connection threads to complete
    for (auto& thread : connection_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    connection_threads_.clear();
    
    LOG_INFO("All shards started");
    return true;
}

void ShardManager::stop() {
    if (!is_running_.load()) {
        return;
    }
    
    is_shutting_down_ = true;
    is_running_ = false;
    
    LOG_INFO("Stopping all shards");
    
    // Disconnect all shards
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& [shard_id, client] : shards_) {
        if (client) {
            client->disconnect();
        }
    }
    
    // Wait for connection threads
    for (auto& thread : connection_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    connection_threads_.clear();
    shards_.clear();
    shard_info_.clear();
    
    LOG_INFO("All shards stopped");
}

bool ShardManager::connect_shard_by_id(int shard_id) {
    if (shard_id < 0 || shard_id >= config_.shard_count) {
        LOG_ERROR("Invalid shard ID: " + std::to_string(shard_id));
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    connect_shard(shard_id);
    return true;
}

void ShardManager::disconnect_shard_by_id(int shard_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    disconnect_shard(shard_id);
}

void ShardManager::reconnect_shard(int shard_id, bool resume) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = shards_.find(shard_id);
    if (it != shards_.end()) {
        disconnect_shard(shard_id);
        
        if (resume && shard_info_[shard_id].is_resumable) {
            // Attempt resume after a short delay
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            connect_shard(shard_id);
        } else {
            // Force identify
            shard_info_[shard_id].session_id.clear();
            shard_info_[shard_id].sequence_number = 0;
            connect_shard(shard_id);
        }
    }
}

void ShardManager::reconnect_all(bool resume) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (int i = 0; i < config_.shard_count; ++i) {
        if (shards_.find(i) != shards_.end()) {
            disconnect_shard(i);
        }
    }
    
    // Wait a bit before reconnecting
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    
    for (int i = 0; i < config_.shard_count; ++i) {
        if (!resume || !shard_info_[i].is_resumable) {
            shard_info_[i].session_id.clear();
            shard_info_[i].sequence_number = 0;
        }
        connect_shard(i);
        
        if (i < config_.shard_count - 1) {
            std::this_thread::sleep_for(config_.connection_delay);
        }
    }
}

bool ShardManager::send_to_shard(int shard_id, const nlohmann::json& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = shards_.find(shard_id);
    if (it != shards_.end() && it->second) {
        it->second->send(event);
        return true;
    }
    
    return false;
}

int ShardManager::send_to_all_shards(const nlohmann::json& event) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int sent_count = 0;
    for (auto& [shard_id, client] : shards_) {
        if (client) {
            client->send(event);
            sent_count++;
        }
    }
    
    return sent_count;
}

const ShardInfo* ShardManager::get_shard_info(int shard_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = shard_info_.find(shard_id);
    if (it != shard_info_.end()) {
        return &it->second;
    }
    
    return nullptr;
}

std::unordered_map<int, ShardInfo> ShardManager::get_all_shard_info() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return shard_info_;
}

int ShardManager::get_connected_shard_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    int count = 0;
    for (const auto& [shard_id, info] : shard_info_) {
        if (info.is_connected) {
            count++;
        }
    }
    
    return count;
}

int ShardManager::get_total_shard_count() const {
    return config_.shard_count;
}

bool ShardManager::is_running() const {
    return is_running_.load();
}

bool ShardManager::is_shard_connected(int shard_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = shard_info_.find(shard_id);
    if (it != shard_info_.end()) {
        return it->second.is_connected;
    }
    
    return false;
}

void ShardManager::set_event_callback(EventCallback callback) {
    event_callback_ = std::move(callback);
}

void ShardManager::set_shard_state_callback(ShardStateCallback callback) {
    shard_state_callback_ = std::move(callback);
}

void ShardManager::set_ready_callback(ReadyCallback callback) {
    ready_callback_ = std::move(callback);
}

void ShardManager::set_config(const ShardConfig& config) {
    if (is_running_.load()) {
        LOG_WARN("Cannot update configuration while ShardManager is running");
        return;
    }
    
    config_ = config;
}

const ShardConfig& ShardManager::get_config() const {
    return config_;
}

const GatewaySession& ShardManager::get_session_info() const {
    return session_info_;
}

void ShardManager::identify_all() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& [shard_id, info] : shard_info_) {
        info.session_id.clear();
        info.sequence_number = 0;
        info.is_resumable = false;
    }
    
    reconnect_all(false);
}

void ShardManager::resume_all() {
    reconnect_all(true);
}

nlohmann::json ShardManager::get_statistics() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    nlohmann::json stats;
    stats["total_shards"] = config_.shard_count;
    stats["connected_shards"] = get_connected_shard_count();
    stats["is_running"] = is_running_.load();
    stats["sessions_started_recently"] = sessions_started_recently_.load();
    
    nlohmann::json shard_stats = nlohmann::json::object();
    for (const auto& [shard_id, info] : shard_info_) {
        nlohmann::json shard_info;
        shard_info["is_connected"] = info.is_connected;
        shard_info["is_resumable"] = info.is_resumable;
        shard_info["reconnect_attempts"] = info.reconnect_attempts;
        shard_info["sequence_number"] = info.sequence_number;
        
        auto now = std::chrono::steady_clock::now();
        auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - info.connect_time);
        shard_info["uptime_seconds"] = uptime.count();
        
        shard_stats[std::to_string(shard_id)] = shard_info;
    }
    
    stats["shards"] = shard_stats;
    return stats;
}

void ShardManager::set_auto_reconnect(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& [shard_id, client] : shards_) {
        if (client) {
            client->enable_auto_reconnect(enabled);
        }
    }
}

void ShardManager::set_reconnection_config(int max_retries,
                                        std::chrono::milliseconds base_delay,
                                        std::chrono::milliseconds max_delay) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (auto& [shard_id, client] : shards_) {
        if (client) {
            client->set_reconnection_config(max_retries, base_delay, max_delay);
        }
    }
}

// Private methods

bool ShardManager::initialize_shard(int shard_id) {
    if (shard_id < 0 || shard_id >= config_.shard_count) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Create shard info if it doesn't exist
    if (shard_info_.find(shard_id) == shard_info_.end()) {
        shard_info_.emplace(shard_id, ShardInfo(shard_id, config_.shard_count));
    }
    
    // Create WebSocket client if it doesn't exist
    if (shards_.find(shard_id) == shards_.end()) {
        auto client = std::make_unique<WebSocketClient>();
        client->set_token(bot_token_);
        client->set_intents(0); // Will be set by caller
        
        // Set up event handlers
        client->on_event([this, shard_id](const nlohmann::json& event) {
            handle_shard_event(shard_id, event);
        });
        
        shards_[shard_id] = std::move(client);
    }
    
    return true;
}

void ShardManager::connect_shard(int shard_id) {
    if (!initialize_shard(shard_id)) {
        return;
    }
    
    auto& shard = shards_[shard_id];
    auto& info = shard_info_[shard_id];
    
    LOG_INFO("Connecting shard " + std::to_string(shard_id));
    
    // Update session start limits
    sessions_started_recently_++;
    update_session_limits();
    
    // Connect to gateway
    std::string url = get_gateway_url();
    if (shard->connect(url)) {
        info.is_connected = true;
        info.connect_time = std::chrono::steady_clock::now();
        info.reconnect_attempts = 0;
        
        // Send identify or resume
        if (info.is_resumable && !info.session_id.empty()) {
            resume_shard(shard_id, nlohmann::json{});
        } else {
            identify_shard(shard_id);
        }
        
        if (shard_state_callback_) {
            shard_state_callback_(shard_id, true);
        }
        
        LOG_INFO("Shard " + std::to_string(shard_id) + " connected successfully");
    } else {
        info.is_connected = false;
        info.reconnect_attempts++;
        
        LOG_ERROR("Failed to connect shard " + std::to_string(shard_id));
        
        if (shard_state_callback_) {
            shard_state_callback_(shard_id, false);
        }
    }
}

void ShardManager::disconnect_shard(int shard_id) {
    auto it = shards_.find(shard_id);
    if (it != shards_.end() && it->second) {
        it->second->disconnect();
        
        auto& info = shard_info_[shard_id];
        info.is_connected = false;
        
        if (shard_state_callback_) {
            shard_state_callback_(shard_id, false);
        }
        
        LOG_INFO("Shard " + std::to_string(shard_id) + " disconnected");
    }
}

void ShardManager::handle_shard_event(int shard_id, const nlohmann::json& event) {
    auto& info = shard_info_[shard_id];
    
    // Update sequence number for dispatch events
    if (event.contains("op") && event["op"] == 0 && event.contains("s")) {
        info.sequence_number = event.value("s", 0);
    }
    
    // Handle specific events
    if (event.contains("t")) {
        std::string event_type = event["t"];
        
        if (event_type == "READY") {
            handle_shard_ready(shard_id, event["d"]);
        } else if (event_type == "RESUMED") {
            handle_shard_resume(shard_id, event["d"]);
        }
    }
    
    // Forward to user callback
    if (event_callback_) {
        event_callback_(shard_id, event);
    }
}

void ShardManager::handle_shard_ready(int shard_id, const nlohmann::json& ready_data) {
    auto& info = shard_info_[shard_id];
    
    info.session_id = ready_data.value("session_id", "");
    info.is_resumable = true;
    info.sequence_number = 0;
    
    LOG_INFO("Shard " + std::to_string(shard_id) + " is ready");
    
    if (ready_callback_) {
        ready_callback_(shard_id, ready_data);
    }
}

void ShardManager::handle_shard_resume(int shard_id, const nlohmann::json& resume_data) {
    auto& info = shard_info_[shard_id];
    
    info.is_resumable = true;
    
    LOG_INFO("Shard " + std::to_string(shard_id) + " resumed successfully");
}

GatewaySession ShardManager::get_gateway_info() {
    try {
        auto response = APIEndpoints::get_gateway_bot();
        
        GatewaySession session;
        session.url = response.value("url", "");
        session.shards = response.value("shards", 1);
        
        if (response.contains("session_start_limit")) {
            auto limit = response["session_start_limit"];
            session.session_start_limit_total = limit.value("total", 1000);
            session.session_start_limit_remaining = limit.value("remaining", 1000);
            session.session_start_limit_reset_after = limit.value("reset_after", 0);
            session.last_reset = std::chrono::steady_clock::now();
        }
        
        return session;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to get gateway info: " + std::string(e.what()));
        
        // Return default session info
        GatewaySession session;
        session.url = "wss://gateway.discord.gg/?v=10&encoding=json";
        session.shards = 1;
        session.session_start_limit_total = 1000;
        session.session_start_limit_remaining = 1000;
        session.session_start_limit_reset_after = 86400000;
        session.last_reset = std::chrono::steady_clock::now();
        
        return session;
    }
}

int ShardManager::calculate_shard_count() {
    if (session_info_.shards > 0) {
        return session_info_.shards;
    }
    
    // Default to 1 shard if we can't determine the optimal count
    return 1;
}

bool ShardManager::can_start_session() {
    update_session_limits();
    return session_info_.can_start_session();
}

void ShardManager::wait_for_session_slot() {
    while (!can_start_session() && !is_shutting_down_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        update_session_limits();
    }
}

void ShardManager::update_session_limits() {
    auto now = std::chrono::steady_clock::now();
    auto time_since_reset = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - session_info_.last_reset);
    
    // Reset if enough time has passed
    if (time_since_reset.count() >= session_info_.session_start_limit_reset_after) {
        session_info_.session_start_limit_remaining = session_info_.session_start_limit_total;
        session_info_.last_reset = now;
        sessions_started_recently_ = 0;
        last_session_reset_ = now;
    }
}

int ShardManager::get_shard_for_guild(const std::string& guild_id) {
    if (config_.shard_count <= 1) {
        return 0;
    }
    
    try {
        uint64_t guild_id_num = std::stoull(guild_id);
        return static_cast<int>((guild_id_num >> 22) % config_.shard_count);
    } catch (const std::exception&) {
        return 0;
    }
}

bool ShardManager::validate_config() const {
    return config_.shard_count > 0 && 
           config_.max_concurrency > 0 &&
           !bot_token_.empty();
}

std::string ShardManager::get_gateway_url() const {
    std::string url = session_info_.url;
    
    if (url.empty()) {
        url = "wss://gateway.discord.gg/?v=10&encoding=json";
    }
    
    // Add compression if enabled
    if (config_.compress) {
        url += "&compress=zlib-stream";
    }
    
    return url;
}

void ShardManager::identify_shard(int shard_id) {
    auto& shard = shards_[shard_id];
    auto& info = shard_info_[shard_id];
    
    nlohmann::json identify;
    identify["op"] = 2;
    identify["d"] = nlohmann::json{
        {"token", bot_token_},
        {"intents", 0}, // Will be set by caller
        {"properties", nlohmann::json{
            {"os", "linux"},
            {"browser", "discord.cpp"},
            {"device", "discord.cpp"}
        }},
        {"shard", nlohmann::json::array({shard_id, config_.shard_count})}
    };
    
    shard->send(identify);
    LOG_DEBUG("Sent IDENTIFY for shard " + std::to_string(shard_id));
}

void ShardManager::resume_shard(int shard_id, const nlohmann::json& resume_data) {
    auto& shard = shards_[shard_id];
    auto& info = shard_info_[shard_id];
    
    nlohmann::json resume;
    resume["op"] = 6;
    resume["d"] = nlohmann::json{
        {"token", bot_token_},
        {"session_id", info.session_id},
        {"seq", info.sequence_number}
    };
    
    shard->send(resume);
    LOG_DEBUG("Sent RESUME for shard " + std::to_string(shard_id));
}

// ShardFactory implementation

ShardConfig ShardFactory::create_small_bot_config() {
    ShardConfig config;
    config.shard_count = 1;
    config.max_concurrency = 1;
    config.connection_delay = std::chrono::milliseconds(5000);
    return config;
}

ShardConfig ShardFactory::create_medium_bot_config() {
    ShardConfig config;
    config.shard_count = 4;
    config.max_concurrency = 2;
    config.connection_delay = std::chrono::milliseconds(2500);
    return config;
}

ShardConfig ShardFactory::create_large_bot_config() {
    ShardConfig config;
    config.shard_count = 16;
    config.max_concurrency = 4;
    config.connection_delay = std::chrono::milliseconds(1000);
    return config;
}

int ShardFactory::calculate_optimal_shards(int guild_count) {
    // Discord recommends 1 shard per 1000-2500 guilds
    const int guilds_per_shard = 2000;
    return std::max(1, (guild_count + guilds_per_shard - 1) / guilds_per_shard);
}

ShardConfig ShardFactory::create_config_for_guild_count(int guild_count) {
    int shard_count = calculate_optimal_shards(guild_count);
    
    ShardConfig config;
    config.shard_count = shard_count;
    config.max_concurrency = std::min(4, shard_count);
    config.connection_delay = std::chrono::milliseconds(std::max(500, 5000 / shard_count));
    
    return config;
}

} // namespace discord