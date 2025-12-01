#pragma once

#include "../core/interfaces.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <vector>
#include <functional>
#include <nlohmann/json.hpp>

namespace discord {

/**
 * @brief Cache entry with TTL support
 */
struct CacheEntry {
    nlohmann::json value;
    std::chrono::system_clock::time_point created_at;
    std::chrono::system_clock::time_point expires_at;
    bool is_persistent;
    
    CacheEntry(
        const nlohmann::json& val,
        std::chrono::seconds ttl = std::chrono::seconds::zero(),
        bool persistent = false
    ) : value(val)
      , created_at(std::chrono::system_clock::now())
      , is_persistent(persistent) {
        
        if (ttl.count() > 0) {
            expires_at = created_at + ttl;
        } else {
            expires_at = std::chrono::system_clock::time_point::max();
        }
    }
    
    bool is_expired() const {
        return !is_persistent && std::chrono::system_clock::now() > expires_at;
    }
};

/**
 * @brief Cache statistics
 */
struct CacheStats {
    size_t total_entries;
    size_t expired_entries;
    size_t persistent_entries;
    size_t memory_usage_bytes;
    std::chrono::system_clock::time_point last_cleanup;
    
    CacheStats() : total_entries(0), expired_entries(0), persistent_entries(0), 
                   memory_usage_bytes(0), last_cleanup(std::chrono::system_clock::now()) {}
};

/**
 * @brief Cache configuration
 */
struct CacheConfig {
    size_t max_entries;
    std::chrono::seconds default_ttl;
    std::chrono::seconds cleanup_interval;
    bool enable_persistence;
    bool enable_compression;
    float cleanup_threshold;
    
    CacheConfig() : max_entries(10000),
                   default_ttl(std::chrono::seconds(3600)),
                   cleanup_interval(std::chrono::seconds(300)),
                   enable_persistence(false),
                   enable_compression(false),
                   cleanup_threshold(0.8f) {}
};

/**
 * @brief Cache Manager implementation for Discord.cpp
 * 
 * Provides thread-safe caching with TTL support, automatic cleanup,
 * and configurable storage strategies. Implements the ICache interface.
 * 
 * @todo TODO.md: Complete missing implementations: CacheManager
 */
class CacheManager : public ICache {
private:
    mutable std::mutex mutex_;
    std::unordered_map<std::string, CacheEntry> cache_;
    CacheConfig config_;
    CacheStats stats_;
    std::vector<std::function<void(const std::string&, const nlohmann::json&)>> eviction_callbacks_;
    std::chrono::system_clock::time_point last_cleanup_;
    bool cleanup_running_;

    /**
     * @brief Estimate memory usage of a JSON value
     * @param value JSON value to estimate
     * @return Estimated memory usage in bytes
     */
    size_t estimate_memory_usage(const nlohmann::json& value) const;

    /**
     * @brief Perform cleanup of expired entries
     * @param force Force cleanup even if not scheduled
     */
    void cleanup_expired(bool force = false);

    /**
     * @brief Evict entries based on LRU policy when cache is full
     * @param required_space Required space to free up
     */
    void evict_lru(size_t required_space = 0);

    /**
     * @brief Check if cleanup should run
     * @return True if cleanup should run
     */
    bool should_cleanup() const;

    /**
     * @brief Update cache statistics
     */
    void update_stats();

    /**
     * @brief Notify eviction callbacks
     * @param key Key being evicted
     * @param value Value being evicted
     */
    void notify_eviction(const std::string& key, const nlohmann::json& value);

    /**
     * @brief Convert wildcard pattern to regex
     * @param pattern Pattern to convert
     * @return Regex string
     */
    std::string pattern_to_regex(const std::string& pattern) const;

public:
    /**
     * @brief Construct CacheManager
     * @param config Cache configuration
     */
    explicit CacheManager(const CacheConfig& config = CacheConfig{});

    /**
     * @brief Destructor - performs final cleanup
     */
    ~CacheManager() override;

    // ICache interface implementation
    void set(const std::string& key, const nlohmann::json& value, std::chrono::seconds ttl = {}) override;
    std::optional<nlohmann::json> get(const std::string& key) override;
    void remove(const std::string& key) override;
    void clear() override;
    bool exists(const std::string& key) override;
    std::vector<std::string> keys(const std::string& pattern = "*") override;

    /**
     * @brief Set cache configuration
     * @param config New configuration
     */
    void set_config(const CacheConfig& config);

    /**
     * @brief Get current cache configuration
     * @return Current configuration
     */
    const CacheConfig& get_config() const;

    /**
     * @brief Get cache statistics
     * @return Current statistics
     */
    CacheStats get_stats() const;

    /**
     * @brief Force cleanup of expired entries
     */
    void force_cleanup();

    /**
     * @brief Add eviction callback
     * @param callback Function to call when entry is evicted
     */
    void add_eviction_callback(std::function<void(const std::string&, const nlohmann::json&)> callback);

    /**
     * @brief Remove eviction callback
     * @param callback Callback to remove
     */
    void remove_eviction_callback(const std::function<void(const std::string&, const nlohmann::json&)>& callback);

    /**
     * @brief Set persistent entry (never expires)
     * @param key Entry key
     * @param value Entry value
     */
    void set_persistent(const std::string& key, const nlohmann::json& value);

    /**
     * @brief Get multiple entries
     * @param keys Vector of keys to retrieve
     * @return Map of found entries
     */
    std::unordered_map<std::string, nlohmann::json> get_multiple(const std::vector<std::string>& keys);

    /**
     * @brief Set multiple entries
     * @param entries Map of key-value pairs to set
     * @param ttl TTL for all entries
     */
    void set_multiple(const std::unordered_map<std::string, nlohmann::json>& entries, 
                     std::chrono::seconds ttl = {});

    /**
     * @brief Remove multiple entries
     * @param keys Vector of keys to remove
     */
    void remove_multiple(const std::vector<std::string>& keys);

    /**
     * @brief Get entries matching pattern
     * @param pattern Pattern to match (supports * wildcards)
     * @return Vector of matching key-value pairs
     */
    std::vector<std::pair<std::string, nlohmann::json>> get_matching(const std::string& pattern);

    /**
     * @brief Get cache size in bytes
     * @return Estimated memory usage
     */
    size_t get_memory_usage() const;

    /**
     * @brief Get number of entries
     * @return Total number of entries
     */
    size_t size() const;

    /**
     * @brief Check if cache is empty
     * @return True if cache is empty
     */
    bool empty() const;

    /**
     * @brief Get TTL for a key
     * @param key Key to check
     * @return TTL in seconds, or 0 if no TTL
     */
    std::chrono::seconds get_ttl(const std::string& key) const;

    /**
     * @brief Update TTL for a key
     * @param key Key to update
     * @param ttl New TTL
     */
    void update_ttl(const std::string& key, std::chrono::seconds ttl);

    /**
     * @brief Export cache to JSON
     * @return JSON representation of cache
     */
    nlohmann::json export_cache() const;

    /**
     * @brief Import cache from JSON
     * @param data JSON data to import
     * @param overwrite Whether to overwrite existing entries
     */
    void import_cache(const nlohmann::json& data, bool overwrite = false);
};

/**
 * @brief Cache factory for creating specialized cache instances
 */
class CacheFactory {
public:
    /**
     * @brief Create memory cache
     * @param config Cache configuration
     * @return Shared pointer to cache instance
     */
    static std::shared_ptr<ICache> create_memory_cache(const CacheConfig& config = CacheConfig{});

    /**
     * @brief Create Redis cache (placeholder for future implementation)
     * @param config Cache configuration
     * @param connection_string Redis connection string
     * @return Shared pointer to cache instance
     */
    static std::shared_ptr<ICache> create_redis_cache(
        const CacheConfig& config = CacheConfig{},
        const std::string& connection_string = ""
    );
};

} // namespace discord