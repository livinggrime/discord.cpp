#include <discord/cache/memory_cache.h>
#include <algorithm>
#include <fnmatch.h>

namespace discord {

void MemoryCache::set(const std::string& key, const nlohmann::json& value, std::chrono::seconds ttl) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    // Default TTL of 1 hour if not specified
    if (ttl == std::chrono::seconds{}) {
        ttl = std::chrono::seconds(3600);
    }
    
    cache_[key] = CacheEntry(value, ttl);
}

std::optional<nlohmann::json> MemoryCache::get(const std::string& key) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return std::nullopt;
    }
    
    if (it->second.is_expired()) {
        // Need to upgrade to unique lock to remove expired entry
        lock.unlock();
        std::unique_lock<std::shared_mutex> unique_lock(mutex_);
        cache_.erase(key);
        return std::nullopt;
    }
    
    return it->second.value;
}

void MemoryCache::remove(const std::string& key) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    cache_.erase(key);
}

void MemoryCache::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    cache_.clear();
}

bool MemoryCache::exists(const std::string& key) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = cache_.find(key);
    if (it == cache_.end()) {
        return false;
    }
    
    if (it->second.is_expired()) {
        lock.unlock();
        std::unique_lock<std::shared_mutex> unique_lock(mutex_);
        cache_.erase(key);
        return false;
    }
    
    return true;
}

std::vector<std::string> MemoryCache::keys(const std::string& pattern) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    cleanup_expired();
    
    if (pattern == "*") {
        std::vector<std::string> all_keys;
        all_keys.reserve(cache_.size());
        for (const auto& pair : cache_) {
            all_keys.push_back(pair.first);
        }
        return all_keys;
    }
    
    return pattern_match(pattern);
}

void MemoryCache::cleanup_expired() {
    // This method should be called with a unique lock held
    auto now = std::chrono::system_clock::now();
    auto it = cache_.begin();
    
    while (it != cache_.end()) {
        if (it->second.is_expired()) {
            it = cache_.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<std::string> MemoryCache::pattern_match(const std::string& pattern) const {
    std::vector<std::string> matches;
    
    for (const auto& pair : cache_) {
        if (fnmatch(pattern.c_str(), pair.first.c_str(), 0) == 0) {
            matches.push_back(pair.first);
        }
    }
    
    return matches;
}

} // namespace discord