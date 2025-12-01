#pragma once

/**
 * @file api.h
 * @brief HTTP API client module
 */

#include "config.h"
#include "api/http_client.h"
#include "api/rest_endpoints.h"
#include "api/rate_limiter.h"

namespace discord::api {
    // Re-export commonly used types
    using discord::HTTPClient;
    using discord::RestEndpoints;
    using discord::RateLimiter;
} // namespace discord::api
