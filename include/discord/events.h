#pragma once

/**
 * @file events.h
 * @brief Event handling and dispatch module
 */

#include "config.h"
#include "events/event_dispatcher.h"
#include "events/event_handlers.h"
#include "events/middleware.h"

namespace discord::events {
    // Re-export commonly used types
    using discord::EventDispatcher;
    using discord::EventHandler;
    using discord::Middleware;
} // namespace discord::events
