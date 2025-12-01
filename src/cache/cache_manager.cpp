#include <discord/cache/cache_manager.h>
#include <discord/utils/logger.h>
#include <discord/core/exceptions.h>
#include <algorithm>
#include <regex>
#include <cstring>

namespace discord {

CacheManager::CacheManager(const CacheConfig& config) 
    : config_(config)
    , last_cleanup_(std::chrono::system_clock::now())
    , cleanup_running_(false) {
    
    LOG_INFO("CacheManager initialized with max_entries: " + std::to_string(config_.max_entries));
}

CacheManager::~CacheManager() {
    std::lock_guard<std::mutex> lock(mutex_);
    cleanup_expired(true);
    LOG_INFO("CacheManager destroyed");
}

void CacheManager::set(const std::string& key, const nlohmann::json& value, std::chrono::seconds ttl) {
    if (key.empty()) {
        throw DiscordException("Cache key cannot be empty");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check if cleanup is needed
    if (should_cleanup()) {
        cleanup_expired();
    }

    // Check if we need to evict entries
    if (cache_.size() >= config_.max_entries) {
        evict_lru();
    }

    // Create new cache entry
    CacheEntry entry(value, ttl.count() > 0 ? ttl : config_.default_ttl);
    cache_[key] = std::move(entry);
    
    update_stats();
    LOG_DEBUG("Cache entry set: " + key);
}

std::optional<nlohmann::json> CacheManager::get(const std::string& key) {
    if (key.empty()) {
        return std::nullopt;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return std::nullopt;
    }

    // Check if entry is expired
    if (it->second.is_expired()) {
        cache_.erase(it);
        update_stats();
        LOG_DEBUG("Cache entry expired and removed: " + key);
        return std::nullopt;
    }

    LOG_DEBUG("Cache entry hit: " + key);
    return it->second.value;
}

void CacheManager::remove(const std::string& key) {
    if (key.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it != cache_.end()) {
        notify_eviction(key, it->second.value);
        cache_.erase(it);
        update_stats();
        LOG_DEBUG("Cache entry removed: " + key);
    }
}

void CacheManager::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Notify all entries before clearing
    for (const auto& [key, entry] : cache_) {
        notify_eviction(key, entry.value);
    }
    
    cache_.clear();
    update_stats();
    LOG_INFO("Cache cleared");
}

bool CacheManager::exists(const std::string& key) {
    if (key.empty()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }

    // Check if entry is expired
    if (it->second.is_expired()) {
        cache_.erase(it);
        update_stats();
        return false;
    }

    return true;
}

std::vector<std::string> CacheManager::keys(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::string> result;
    std::regex regex_pattern(pattern_to_regex(pattern));
    
    for (const auto& [key, entry] : cache_) {
        if (!entry.is_expired() && std::regex_match(key, regex_pattern)) {
            result.push_back(key);
        }
    }
    
    return result;
}

void CacheManager::set_config(const CacheConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    config_ = config;
    
    // If max entries decreased, evict excess entries
    if (cache_.size() > config_.max_entries) {
        evict_lru(cache_.size() - config_.max_entries);
    }
    
    LOG_INFO("Cache configuration updated");
}

const CacheConfig& CacheManager::get_config() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return config_;
}

CacheStats CacheManager::get_stats() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_;
}

void CacheManager::force_cleanup() {
    std::lock_guard<std::mutex> lock(mutex_);
    cleanup_expired(true);
}

void CacheManager::add_eviction_callback(std::function<void(const std::string&, const nlohmann::json&)> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    eviction_callbacks_.push_back(std::move(callback));
}

void CacheManager::remove_eviction_callback(const std::function<void(const std::string&, const nlohmann::json&)>& callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    eviction_callbacks_.erase(
        std::remove_if(eviction_callbacks_.begin(), eviction_callbacks_.end(),
            [&callback](const auto& cb) { 
                // Simple comparison by target address (not perfect but functional)
                return cb.target<void(*)(const std::string&, const nlohmann::json&)>() == 
                       callback.target<void(*)(const std::string&, const nlohmann::json&)>(); 
            }),
        eviction_callbacks_.end()
    );
}

void CacheManager::set_persistent(const std::string& key, const nlohmann::json& value) {
    if (key.empty()) {
        throw DiscordException("Cache key cannot be empty");
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    if (should_cleanup()) {
        cleanup_expired();
    }

    if (cache_.size() >= config_.max_entries) {
        evict_lru();
    }

    CacheEntry entry(value, std::chrono::seconds::zero(), true);
    cache_[key] = std::move(entry);
    
    update_stats();
    LOG_DEBUG("Persistent cache entry set: " + key);
}

std::unordered_map<std::string, nlohmann::json> CacheManager::get_multiple(const std::vector<std::string>& keys) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::unordered_map<std::string, nlohmann::json> result;
    
    for (const auto& key : keys) {
        auto it = cache_.find(key);
        if (it != cache_.end() && !it->second.is_expired()) {
            result[key] = it->second.value;
        }
    }
    
    return result;
}

void CacheManager::set_multiple(const std::unordered_map<std::string, nlohmann::json>& entries, 
                               std::chrono::seconds ttl) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (should_cleanup()) {
        cleanup_expired();
    }

    // Check if we need to evict entries
    size_t required_space = entries.size();
    if (cache_.size() + required_space > config_.max_entries) {
        evict_lru(required_space);
    }

    // Add all entries
    for (const auto& [key, value] : entries) {
        if (!key.empty()) {
            CacheEntry entry(value, ttl.count() > 0 ? ttl : config_.default_ttl);
            cache_[key] = std::move(entry);
        }
    }
    
    update_stats();
    LOG_DEBUG("Multiple cache entries set: " + std::to_string(entries.size()));
}

void CacheManager::remove_multiple(const std::vector<std::string>& keys) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& key : keys) {
        auto it = cache_.find(key);
        if (it != cache_.end()) {
            notify_eviction(key, it->second.value);
            cache_.erase(it);
        }
    }
    
    update_stats();
    LOG_DEBUG("Multiple cache entries removed: " + std::to_string(keys.size()));
}

std::vector<std::pair<std::string, nlohmann::json>> CacheManager::get_matching(const std::string& pattern) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    std::vector<std::pair<std::string, nlohmann::json>> result;
    std::regex regex_pattern(pattern_to_regex(pattern));
    
    for (const auto& [key, entry] : cache_) {
        if (!entry.is_expired() && std::regex_match(key, regex_pattern)) {
            result.emplace_back(key, entry.value);
        }
    }
    
    return result;
}

size_t CacheManager::get_memory_usage() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return stats_.memory_usage_bytes;
}

size_t CacheManager::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.size();
}

bool CacheManager::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return cache_.empty();
}

std::chrono::seconds CacheManager::get_ttl(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it == cache_.end() || it->second.is_expired()) {
        return std::chrono::seconds(0);
    }
    
    if (it->second.is_persistent) {
        return std::chrono::seconds::max();
    }
    
    auto now = std::chrono::system_clock::now();
    if (it->second.expires_at <= now) {
        return std::chrono::seconds(0);
    }
    
    return std::chrono::duration_cast<std::chrono::seconds>(it->second.expires_at - now);
}

void CacheManager::update_ttl(const std::string& key, std::chrono::seconds ttl) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it != cache_.end() && !it->second.is_persistent) {
        if (ttl.count() > 0) {
            it->second.expires_at = std::chrono::system_clock::now() + ttl;
        } else {
            it->second.expires_at = std::chrono::system_clock::time_point::max();
        }
        LOG_DEBUG("TTL updated for key: " + key);
    }
}

nlohmann::json CacheManager::export_cache() const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    nlohmann::json export_data;
    export_data["entries"] = nlohmann::json::object();
    
    for (const auto& [key, entry] : cache_) {
        if (!entry.is_expired()) {
            nlohmann::json entry_data;
            entry_data["value"] = entry.value;
            entry_data["created_at"] = std::chrono::duration_cast<std::chrono::seconds>(
                entry.created_at.time_since_epoch()).count();
            entry_data["expires_at"] = std::chrono::duration_cast<std::chrono::seconds>(
                entry.expires_at.time_since_epoch()).count();
            entry_data["is_persistent"] = entry.is_persistent;
            
            export_data["entries"][key] = entry_data;
        }
    }
    
    export_data["config"] = {
        {"max_entries", config_.max_entries},
        {"default_ttl", config_.default_ttl.count()},
        {"cleanup_interval", config_.cleanup_interval.count()},
        {"enable_persistence", config_.enable_persistence},
        {"enable_compression", config_.enable_compression},
        {"cleanup_threshold", config_.cleanup_threshold}
    };
    
    return export_data;
}

void CacheManager::import_cache(const nlohmann::json& data, bool overwrite) {
    if (!data.contains("entries")) {
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    
    for (const auto& [key, entry_data] : data["entries"].items()) {
        if (key.empty()) {
            continue;
        }

        if (!overwrite && cache_.find(key) != cache_.end()) {
            continue;
        }

        try {
            nlohmann::json value = entry_data["value"];
            bool is_persistent = entry_data.value("is_persistent", false);
            
            auto created_at = std::chrono::system_clock::time_point(
                std::chrono::seconds(entry_data.value("created_at", 0)));
            auto expires_at = std::chrono::system_clock::time_point(
                std::chrono::seconds(entry_data.value("expires_at", 0)));
            
            CacheEntry entry(value);
            entry.created_at = created_at;
            entry.expires_at = expires_at;
            entry.is_persistent = is_persistent;
            
            cache_[key] = std::move(entry);
        } catch (const std::exception& e) {
            LOG_ERROR("Failed to import cache entry " + key + ": " + e.what());
        }
    }
    
    update_stats();
    LOG_INFO("Cache import completed");
}

// Private methods

size_t CacheManager::estimate_memory_usage(const nlohmann::json& value) const {
    // Rough estimation - in a real implementation, this would be more accurate
    std::string json_str = value.dump();
    return json_str.size() + sizeof(CacheEntry) + sizeof(std::string);
}

void CacheManager::cleanup_expired(bool force) {
    if (!force && !should_cleanup()) {
        return;
    }

    size_t removed_count = 0;
    auto it = cache_.begin();
    
    while (it != cache_.end()) {
        if (it->second.is_expired()) {
            notify_eviction(it->first, it->second.value);
            it = cache_.erase(it);
            removed_count++;
        } else {
            ++it;
        }
    }
    
    last_cleanup_ = std::chrono::system_clock::now();
    update_stats();
    
    if (removed_count > 0) {
        LOG_DEBUG("Cleaned up " + std::to_string(removed_count) + " expired cache entries");
    }
}

void CacheManager::evict_lru(size_t required_space) {
    if (cache_.empty()) {
        return;
    }

    // Simple LRU implementation - in practice, you'd want to track access times
    size_t to_remove = required_space > 0 ? required_space : 
                       static_cast<size_t>(cache_.size() * config_.cleanup_threshold);
    
    size_t removed = 0;
    auto it = cache_.begin();
    
    while (it != cache_.end() && removed < to_remove) {
        // Skip persistent entries during eviction
        if (!it->second.is_persistent) {
            notify_eviction(it->first, it->second.value);
            it = cache_.erase(it);
            removed++;
        } else {
            ++it;
        }
    }
    
    update_stats();
    LOG_DEBUG("Evicted " + std::to_string(removed) + " cache entries");
}

bool CacheManager::should_cleanup() const {
    auto now = std::chrono::system_clock::now();
    return (now - last_cleanup_) >= config_.cleanup_interval;
}

void CacheManager::update_stats() {
    stats_.total_entries = cache_.size();
    stats_.expired_entries = 0;
    stats_.persistent_entries = 0;
    stats_.memory_usage_bytes = 0;
    
    for (const auto& [key, entry] : cache_) {
        if (entry.is_expired()) {
            stats_.expired_entries++;
        }
        if (entry.is_persistent) {
            stats_.persistent_entries++;
        }
        stats_.memory_usage_bytes += key.size() + estimate_memory_usage(entry.value);
    }
    
    stats_.last_cleanup = last_cleanup_;
}

void CacheManager::notify_eviction(const std::string& key, const nlohmann::json& value) {
    for (const auto& callback : eviction_callbacks_) {
        try {
            callback(key, value);
        } catch (const std::exception& e) {
            LOG_ERROR("Eviction callback error: " + std::string(e.what()));
        }
    }
}

std::string CacheManager::pattern_to_regex(const std::string& pattern) const {
    // Convert simple wildcard pattern to regex
    std::string regex_pattern = pattern;
    std::replace(regex_pattern.begin(), regex_pattern.end(), '*', ".*");
    std::replace(regex_pattern.begin(), regex_pattern.end(), '?', ".");
    return "^" + regex_pattern + "$";
}

// CacheFactory implementation

std::shared_ptr<ICache> CacheFactory::create_memory_cache(const CacheConfig& config) {
    return std::make_shared<CacheManager>(config);
}

std::shared_ptr<ICache> CacheFactory::create_redis_cache(
    const CacheConfig& config,
    const std::string& connection_string
) {
    // TODO: Implement Redis cache when Redis support is added
    // For now, return memory cache as fallback
    LOG_WARN("Redis cache not implemented yet, falling back to memory cache");
    return create_memory_cache(config);
}

} // namespace discord