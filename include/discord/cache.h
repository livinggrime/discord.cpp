#pragma once

/**
 * @file cache.h
 * @brief Caching layer module
 */

#include "config.h"
#include "cache/cache_manager.h"
#include "cache/memory_cache.h"
#include "cache/redis_cache.h"

namespace discord::cache {
    // Re-export commonly used types
    using discord::CacheManager;
    using discord::MemoryCache;
    using discord::RedisCache;
} // namespace discord::cache
