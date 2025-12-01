#pragma once

/**
 * @file core.h
 * @brief Core Discord client and utilities module
 */

#include "config.h"
#include "core/exceptions.h"
#include "core/interfaces.h"
#include "core/client.h"

namespace discord::core {
    // Re-export commonly used types and exceptions
    using discord::ErrorCode;
    using discord::DiscordException;
    using discord::HTTPException;
    using discord::WebSocketException;
    using discord::AuthenticationException;
    using discord::RateLimitException;
    using discord::ValidationException;
} // namespace discord::core
