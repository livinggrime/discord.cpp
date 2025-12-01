#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <nlohmann/json.hpp>
#include "websocket_client.h"
#include "reconnection.h"

namespace discord {

/**
 * @brief Shard information and state
 */
struct ShardInfo {
    int shard_id;
    int shard_count;
    std::string session_id;
    int sequence_number;
    bool is_connected;
    bool is_resumable;
    std::chrono::steady_clock::time_point last_heartbeat;
    std::chrono::steady_clock::time_point connect_time;
    int reconnect_attempts;
    
    ShardInfo(int id, int total) 
        : shard_id(id), shard_count(total), sequence_number(0), 
          is_connected(false), is_resumable(false), reconnect_attempts(0) {}
};

/**
 * @brief Shard configuration
 */
struct ShardConfig {
    int shard_count;
    int max_concurrency;
    std::chrono::milliseconds connection_delay;
    std::chrono::milliseconds heartbeat_interval;
    bool auto_sharding;
    bool compress;
    
    ShardConfig() 
        : shard_count(1), max_concurrency(1), 
          connection_delay(std::chrono::milliseconds(5000)),
          heartbeat_interval(std::chrono::milliseconds(41250)),
          auto_sharding(true), compress(true) {}
};

/**
 * @brief Gateway session information
 */
struct GatewaySession {
    std::string url;
    int shards;
    int session_start_limit_total;
    int session_start_limit_remaining;
    int session_start_limit_reset_after;
    std::chrono::steady_clock::time_point last_reset;
    
    bool can_start_session() const {
        return session_start_limit_remaining > 0;
    }
};

/**
 * @brief Shard Manager for Discord.cpp
 * 
 * Manages multiple WebSocket connections (shards) for large bots,
 * handling auto-sharding, load balancing, and session management.
 * 
 * @todo TODO.md: Implement Shard Manager with auto-sharding and load balancing
 */
class ShardManager {
public:
    using EventCallback = std::function<void(int, const nlohmann::json&)>;
    using ShardStateCallback = std::function<void(int, bool)>;
    using ReadyCallback = std::function<void(int, const nlohmann::json&)>;

private:
    ShardConfig config_;
    std::unordered_map<int, std::unique_ptr<WebSocketClient>> shards_;
    std::unordered_map<int, ShardInfo> shard_info_;
    std::string gateway_url_;
    std::string bot_token_;
    
    // Callbacks
    EventCallback event_callback_;
    ShardStateCallback shard_state_callback_;
    ReadyCallback ready_callback_;
    
    // Threading and synchronization
    mutable std::mutex mutex_;
    std::vector<std::thread> connection_threads_;
    std::atomic<bool> is_running_{false};
    std::atomic<bool> is_shutting_down_{false};
    
    // Session management
    GatewaySession session_info_;
    std::chrono::steady_clock::time_point last_session_update_;
    
    // Rate limiting
    std::atomic<int> sessions_started_recently_{0};
    std::chrono::steady_clock::time_point last_session_reset_;

    /**
     * @brief Initialize a single shard
     * @param shard_id Shard ID to initialize
     * @return True if initialization was successful
     */
    bool initialize_shard(int shard_id);

    /**
     * @brief Connect a single shard
     * @param shard_id Shard ID to connect
     */
    void connect_shard(int shard_id);

    /**
     * @brief Disconnect a single shard
     * @param shard_id Shard ID to disconnect
     */
    void disconnect_shard(int shard_id);

    /**
     * @brief Handle shard connection events
     * @param shard_id Shard ID
     * @param event Event data
     */
    void handle_shard_event(int shard_id, const nlohmann::json& event);

    /**
     * @brief Handle shard disconnection
     * @param shard_id Shard ID
     * @param close_code Close code
     * @param reason Close reason
     */
    void handle_shard_disconnect(int shard_id, int close_code, const std::string& reason);

    /**
     * @brief Handle shard ready event
     * @param shard_id Shard ID
     * @param ready_data Ready event data
     */
    void handle_shard_ready(int shard_id, const nlohmann::json& ready_data);

    /**
     * @brief Handle shard resume
     * @param shard_id Shard ID
     * @param resume_data Resume event data
     */
    void handle_shard_resume(int shard_id, const nlohmann::json& resume_data);

    /**
     * @brief Get gateway information from Discord
     * @return Gateway session information
     */
    GatewaySession get_gateway_info();

    /**
     * @brief Calculate optimal shard count
     * @return Recommended shard count
     */
    int calculate_shard_count();

    /**
     * @brief Check if we can start a new session
     * @return True if session can be started
     */
    bool can_start_session();

    /**
     * @brief Wait for session slot
     */
    void wait_for_session_slot();

    /**
     * @brief Update session start limits
     */
    void update_session_limits();

    /**
     * @brief Get shard for guild ID
     * @param guild_id Guild ID
     * @return Shard ID that should handle this guild
     */
    int get_shard_for_guild(const std::string& guild_id);

    /**
     * @brief Validate shard configuration
     * @return True if configuration is valid
     */
    bool validate_config() const;

    /**
     * @brief Get gateway URL with parameters
     * @return Formatted gateway URL
     */
    std::string get_gateway_url() const;

    /**
     * @brief Send identify payload for specific shard
     * @param shard_id Shard ID to identify
     */
    void identify_shard(int shard_id);

    /**
     * @brief Send resume payload for specific shard
     * @param shard_id Shard ID to resume
     * @param resume_data Resume data (optional)
     */
    void resume_shard(int shard_id, const nlohmann::json& resume_data);

public:
    /**
     * @brief Construct ShardManager
     * @param token Bot token
     * @param config Shard configuration
     */
    explicit ShardManager(const std::string& token, const ShardConfig& config = ShardConfig{});

    /**
     * @brief Destructor - ensures clean shutdown
     */
    ~ShardManager();

    /**
     * @brief Start all shards
     * @return True if all shards started successfully
     */
    bool start();

    /**
     * @brief Stop all shards
     */
    void stop();

    /**
     * @brief Connect specific shard
     * @param shard_id Shard ID to connect
     * @return True if connection initiated successfully
     */
    bool connect_shard_by_id(int shard_id);

    /**
     * @brief Disconnect specific shard
     * @param shard_id Shard ID to disconnect
     */
    void disconnect_shard_by_id(int shard_id);

    /**
     * @brief Reconnect specific shard
     * @param shard_id Shard ID to reconnect
     * @param resume Whether to attempt resume
     */
    void reconnect_shard(int shard_id, bool resume = true);

    /**
     * @brief Reconnect all shards
     * @param resume Whether to attempt resume for all shards
     */
    void reconnect_all(bool resume = true);

    /**
     * @brief Send event to specific shard
     * @param shard_id Target shard ID
     * @param event Event to send
     * @return True if sent successfully
     */
    bool send_to_shard(int shard_id, const nlohmann::json& event);

    /**
     * @brief Send event to all shards
     * @param event Event to send
     * @return Number of shards event was sent to
     */
    int send_to_all_shards(const nlohmann::json& event);

    /**
     * @brief Get shard information
     * @param shard_id Shard ID
     * @return Shard information or nullptr if not found
     */
    const ShardInfo* get_shard_info(int shard_id) const;

    /**
     * @brief Get all shard information
     * @return Map of shard ID to shard information
     */
    std::unordered_map<int, ShardInfo> get_all_shard_info() const;

    /**
     * @brief Get number of connected shards
     * @return Number of connected shards
     */
    int get_connected_shard_count() const;

    /**
     * @brief Get total number of shards
     * @return Total shard count
     */
    int get_total_shard_count() const;

    /**
     * @brief Check if shard manager is running
     * @return True if running
     */
    bool is_running() const;

    /**
     * @brief Check if specific shard is connected
     * @param shard_id Shard ID
     * @return True if connected
     */
    bool is_shard_connected(int shard_id) const;

    /**
     * @brief Set event callback
     * @param callback Function to call for events
     */
    void set_event_callback(EventCallback callback);

    /**
     * @brief Set shard state callback
     * @param callback Function to call when shard state changes
     */
    void set_shard_state_callback(ShardStateCallback callback);

    /**
     * @brief Set ready callback
     * @param callback Function to call when shard becomes ready
     */
    void set_ready_callback(ReadyCallback callback);

    /**
     * @brief Update shard configuration
     * @param config New configuration
     */
    void set_config(const ShardConfig& config);

    /**
     * @brief Get current configuration
     * @return Current shard configuration
     */
    const ShardConfig& get_config() const;

    /**
     * @brief Get gateway session information
     * @return Current session information
     */
    const GatewaySession& get_session_info() const;

    /**
     * @brief Force identify all shards
     */
    void identify_all();

    /**
     * @brief Force resume all shards
     */
    void resume_all();

    /**
     * @brief Get shard statistics
     * @return JSON object with shard statistics
     */
    nlohmann::json get_statistics() const;

    /**
     * @brief Enable/disable auto-reconnect for all shards
     * @param enabled Whether to enable auto-reconnect
     */
    void set_auto_reconnect(bool enabled);

    /**
     * @brief Set reconnection configuration for all shards
     * @param max_retries Maximum reconnection attempts
     * @param base_delay Base delay between attempts
     * @param max_delay Maximum delay between attempts
     */
    void set_reconnection_config(int max_retries = 5,
                               std::chrono::milliseconds base_delay = std::chrono::milliseconds(1000),
                               std::chrono::milliseconds max_delay = std::chrono::milliseconds(30000));
};

/**
 * @brief Shard factory for creating optimized shard configurations
 */
class ShardFactory {
public:
    /**
     * @brief Create configuration for small bots
     * @return Configuration optimized for small bots
     */
    static ShardConfig create_small_bot_config();

    /**
     * @brief Create configuration for medium bots
     * @return Configuration optimized for medium bots
     */
    static ShardConfig create_medium_bot_config();

    /**
     * @brief Create configuration for large bots
     * @return Configuration optimized for large bots
     */
    static ShardConfig create_large_bot_config();

    /**
     * @brief Calculate optimal shard count based on guild count
     * @param guild_count Number of guilds bot is in
     * @return Recommended shard count
     */
    static int calculate_optimal_shards(int guild_count);

    /**
     * @brief Create configuration based on guild count
     * @param guild_count Number of guilds bot is in
     * @return Optimized configuration
     */
    static ShardConfig create_config_for_guild_count(int guild_count);
};

} // namespace discord