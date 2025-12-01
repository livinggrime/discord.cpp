#pragma once

/**
 * @file gateway.h
 * @brief WebSocket gateway and real-time events module
 */

#include "config.h"
#include "gateway/websocket_client.h"
#include "gateway/gateway_events.h"
#include "gateway/reconnection.h"
#include "gateway/shard_manager.h"

namespace discord::gateway {
    // Re-export commonly used types
    using discord::WebSocketClient;
    using discord::GatewayOpcode;
    using discord::GatewayCloseEvent;
    using discord::ReconnectionManager;
    using discord::ShardManager;
} // namespace discord::gateway
