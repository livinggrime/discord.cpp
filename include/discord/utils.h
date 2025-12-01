#pragma once

/**
 * @file utils.h
 * @brief Utility module containing helpers and common utilities
 */

#include "config.h"
#include "utils/types.h"
#include "utils/logger.h"
#include "utils/thread_pool.h"
#include "utils/config_manager.h"
#include "utils/auth.h"
#include "utils/embed_builder.h"

namespace discord::utils {
    // Re-export commonly used types
    template<typename T>
    using Result = discord::Result<T>;
    
    using discord::Logger;
    using discord::ThreadPool;
    using discord::ConfigManager;
    using discord::Auth;
    using discord::EmbedBuilder;
} // namespace discord::utils
