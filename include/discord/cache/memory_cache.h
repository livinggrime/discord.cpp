#pragma once

#include "../core/interfaces.h"
#include <unordered_map>
#include <shared_mutex>
#include <chrono>
#include <optional>
#include <vector>
#include <nlohmann/json.hpp>

namespace discord {

struct CacheEntry {
    nlohmann::json value;
    std::chrono::system_clock::time_point expiry;
    
    CacheEntry(const nlohmann::json& val, std::chrono::seconds ttl)
        : value(val), expiry(std::chrono::system_clock::now() + ttl) {}
    
    bool is_expired() const {
        return std::chrono::system_clock::now() > expiry;
    }
};

class MemoryCache : public ICache {
private:
    std::unordered_map<std::string, CacheEntry> cache_;
    mutable std::shared_mutex mutex_;
    
public:
    MemoryCache() = default;
    ~MemoryCache() override = default;
    
    void set(const std::string& key, const nlohmann::json& value, std::chrono::seconds ttl = {}) override;
    std::optional<nlohmann::json> get(const std::string& key) override;
    void remove(const std::string& key) override;
    void clear() override;
    bool exists(const std::string& key) override;
    std::vector<std::string> keys(const std::string& pattern = "*") override;
    
private:
    void cleanup_expired();
    std::vector<std::string> pattern_match(const std::string& pattern) const;
};

} // namespace discord