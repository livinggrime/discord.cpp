#pragma once

/**
 * @file discord.h
 * @brief Main discord.cpp header - includes all public APIs
 * 
 * This header provides a single entry point for all discord.cpp functionality.
 * Individual module headers can be included for finer-grained control.
 */

// Version and configuration
#include "config.h"

// Core module (exceptions, interfaces, main client)
#include "core.h"

// API module (HTTP client, REST endpoints, rate limiting)
#include "api.h"

// Gateway module (WebSocket, real-time events)
#include "gateway.h"

// Events module (event handling and dispatch)
#include "events.h"

// Cache module (caching layer)
#include "cache.h"

// Utilities module (common utilities and helpers)
#include "utils.h"

// Commands module (command handling)
#include "commands.h"

namespace discord {
    /**
     * @brief Get library version
     * @return Version string in format "major.minor.patch"
     */
    inline const char* version() {
        return "1.0.0";
    }
    
    /**
     * @brief Get build information
     * @return Build information string
     */
    inline const char* build_info() {
        return "discord.cpp v1.0.0 - Modern C++ Discord API wrapper";
    }
} // namespace discord
