# discord.cpp - Modular Architecture

## Overview

discord.cpp is a modern C++23 Discord API wrapper featuring a clean, modular architecture with well-organized namespaces and clear separation of concerns.

## Directory Structure

```
discord.cpp/
├── CMakeLists.txt              # Root CMake configuration with build options
├── include/
│   └── discord/
│       ├── discord.h           # Main public header (include all modules)
│       ├── config.h            # Version and feature configuration
│       ├── core.h              # Core module header
│       ├── api.h               # API module header
│       ├── gateway.h           # Gateway module header
│       ├── events.h            # Events module header
│       ├── cache.h             # Cache module header
│       ├── utils.h             # Utils module header
│       ├── commands.h          # Commands module header
│       ├── core/               # Core implementation headers
│       │   ├── client.h
│       │   ├── exceptions.h
│       │   └── interfaces.h
│       ├── api/                # API implementation headers
│       │   ├── http_client.h
│       │   ├── rest_endpoints.h
│       │   └── rate_limiter.h
│       ├── gateway/            # Gateway implementation headers
│       │   ├── websocket_client.h
│       │   ├── gateway_events.h
│       │   ├── reconnection.h
│       │   └── shard_manager.h
│       ├── events/             # Events implementation headers
│       │   ├── event_dispatcher.h
│       │   ├── event_handlers.h
│       │   └── middleware.h
│       ├── cache/              # Cache implementation headers
│       │   ├── cache_manager.h
│       │   ├── memory_cache.h
│       │   └── redis_cache.h
│       ├── utils/              # Utilities implementation headers
│       │   ├── types.h
│       │   ├── logger.h
│       │   ├── thread_pool.h
│       │   ├── config_manager.h
│       │   ├── auth.h
│       │   └── embed_builder.h
│       └── commands/           # Commands implementation headers
│           └── command_manager.h
├── src/
│   ├── CMakeLists.txt          # Library target configuration
│   ├── core/                   # Core module implementations
│   ├── api/                    # API module implementations
│   ├── gateway/                # Gateway module implementations
│   ├── events/                 # Events module implementations
│   ├── cache/                  # Cache module implementations
│   └── utils/                  # Utils module implementations
└── examples/
    └── discord_py_like_working_bot.cpp
```

## Module Organization

### Namespace Hierarchy

The codebase is organized into clear namespaces for better organization and to prevent naming conflicts:

```cpp
discord::
├── core::         // Core client and exceptions
├── api::          // HTTP and REST APIs
├── gateway::      // WebSocket and real-time events
├── events::       // Event handling and dispatch
├── cache::        // Data caching layer
├── utils::        // Common utilities and helpers
└── commands::     // Command parsing and management
```

### Module Descriptions

#### Core Module (`discord::core`)
- **Purpose**: Main Discord client and error handling
- **Key Files**: `client.h`, `exceptions.h`, `interfaces.h`
- **Responsibilities**: 
  - Provides the main `DiscordClient` class
  - Defines core exception hierarchy
  - Declares interface contracts for other modules

#### API Module (`discord::api`)
- **Purpose**: HTTP communication and REST endpoints
- **Key Files**: `http_client.h`, `rest_endpoints.h`, `rate_limiter.h`
- **Responsibilities**:
  - Manages HTTP requests using CURL
  - Implements Discord REST API endpoints
  - Handles API rate limiting

#### Gateway Module (`discord::gateway`)
- **Purpose**: WebSocket connection and real-time events
- **Key Files**: `websocket_client.h`, `gateway_events.h`, `reconnection.h`, `shard_manager.h`
- **Responsibilities**:
  - WebSocket connection management
  - Gateway event handling
  - Automatic reconnection logic
  - Shard management for large bots

#### Events Module (`discord::events`)
- **Purpose**: Event dispatch and handling system
- **Key Files**: `event_dispatcher.h`, `event_handlers.h`, `middleware.h`
- **Responsibilities**:
  - Event registration and dispatch
  - Event handler execution
  - Middleware support for event processing

#### Cache Module (`discord::cache`)
- **Purpose**: Data caching layer
- **Key Files**: `cache_manager.h`, `memory_cache.h`, `redis_cache.h`
- **Responsibilities**:
  - In-memory caching of Discord data
  - Redis-backed caching for distributed systems
  - Cache invalidation and TTL management

#### Utils Module (`discord::utils`)
- **Purpose**: Utility functions and common helpers
- **Key Files**: 
  - `types.h` - Generic Result type and common structures
  - `logger.h` - Logging utilities
  - `thread_pool.h` - Thread pool for async tasks
  - `config_manager.h` - Configuration management
  - `auth.h` - Authentication utilities
  - `embed_builder.h` - Discord embed building utilities
- **Responsibilities**: Provide reusable components for the entire library

#### Commands Module (`discord::commands`)
- **Purpose**: Command parsing and management
- **Key Files**: `command_manager.h`
- **Responsibilities**:
  - Command registration
  - Command parsing and dispatch
  - Permission checking

## Include Strategies

### For Library Users

Include the main header for all functionality:
```cpp
#include <discord.h>
```

Or include specific modules:
```cpp
#include <discord/core.h>
#include <discord/api.h>
#include <discord/events.h>
```

### For Library Developers

Include specific implementation headers:
```cpp
#include <discord/core/client.h>
#include <discord/api/http_client.h>
```

## Dependency Flow

The modules follow this dependency hierarchy (lower modules depend on those above):

```
top-level APIs
     ↓
[Events] → [Gateway] → [Cache]
     ↓          ↓          ↓
[Commands]     [API]    [Utils]
     ↓          ↓          ↓
[Core]────────────────────────
     ↓
[Interfaces]
```

### Design Principles

1. **Clear Dependencies**: Each module clearly declares its dependencies
2. **Interface Segregation**: Interfaces define contracts without implementation details
3. **Namespace Isolation**: Namespaces prevent naming conflicts
4. **Header Organization**: Public headers re-export commonly used types
5. **Forward Declarations**: Reduce coupling between modules

## CMake Configuration

### Build Options

```cmake
-DBUILD_SHARED_LIBS=ON/OFF      # Build as shared or static library
-DBUILD_EXAMPLES=ON/OFF         # Build example programs
-DBUILD_TESTS=ON/OFF            # Build and enable tests
-DENABLE_CODE_COVERAGE=ON/OFF   # Enable code coverage analysis
```

### Library Target

The main target is `discord_cpp` with:
- C++23 standard requirement
- Public headers in `include/`
- Linked dependencies: nlohmann_json, OpenSSL, CURL, websocketpp (optional)

## Example Usage

```cpp
#include <discord.h>
#include <iostream>

int main() {
    // Create client using the modular API
    discord::DiscordClient client("YOUR_TOKEN_HERE");
    
    // Use specific modules
    client.on_ready([]() {
        std::cout << "Bot ready!" << std::endl;
    });
    
    client.on_message([](const nlohmann::json& msg) {
        if (msg["content"] == "!ping") {
            // Handle message
        }
    });
    
    client.run();
    return 0;
}
```

## Adding New Modules

To add a new module:

1. Create headers in `include/discord/mymodule/`
2. Create implementations in `src/mymodule/`
3. Update `src/CMakeLists.txt` with new source files
4. Create module header: `include/discord/mymodule.h`
5. Update main header: `include/discord.h`
6. Document namespace: `discord::mymodule::`

## Build Instructions

```bash
# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_EXAMPLES=ON

# Build
cmake --build . --config Release

# Run example
./bin/discord_py_like_bot
```

## Code Organization Best Practices

1. **One Responsibility**: Each module should have a clear, single responsibility
2. **Interface First**: Define interfaces before implementations
3. **Namespace Scoping**: Use namespaces to organize related functionality
4. **Forward Declarations**: Use forward declarations to minimize coupling
5. **Header Hierarchy**: Public headers re-export, implementation headers include details
