# Migration Guide

## Updating Your Code to Use the New Modular Structure

This guide helps you migrate from the old codebase structure to the new modular organization.

## What's Changed

### Before (Old Structure)
```cpp
#include "discord/client.h"
#include "discord/http_client.h"
#include "discord/websocket_client.h"

// Everything in global discord namespace
discord::DiscordClient client(token);
```

### After (New Structure)
```cpp
#include <discord.h>  // Single entry point, or specific modules

// Everything organized in sub-namespaces
discord::DiscordClient client(token);  // Still works!
// Now also available as: discord::core::DiscordClient
```

## Migration Steps

### Step 1: Update Your Include Statements

#### Option A: Use Single Main Header (Recommended)
```cpp
// Old way (might still work)
#include "discord/client.h"
#include "discord/api/http_client.h"
#include "discord/events/event_dispatcher.h"

// New way (recommended)
#include <discord.h>
```

#### Option B: Use Module Headers
```cpp
// Specific modules if you don't need everything
#include <discord/core.h>
#include <discord/api.h>
#include <discord/events.h>
```

#### Option C: Specific Components (if you need fine-grained control)
```cpp
// Still possible with the new structure
#include <discord/core/client.h>
#include <discord/api/http_client.h>
```

### Step 2: Update CMakeLists.txt

#### Before
```cmake
find_package(discord REQUIRED)
target_link_libraries(myapp discord)
```

#### After
```cmake
# Same API, but now with better organization
find_package(discord_cpp REQUIRED)
target_link_libraries(myapp discord_cpp::discord_cpp)

# Alternative
# Just link the library and it includes everything
target_link_libraries(myapp discord_cpp)
```

### Step 3: Update Namespace References

#### If using explicit namespaces
```cpp
// Old - still works
discord::DiscordClient client(token);

// New option - explicit subnamespace
discord::core::DiscordClient client(token);  // More explicit

// New option - using module namespaces
discord::api::HTTPClient http_client(token);
discord::gateway::WebSocketClient ws_client;
discord::events::EventDispatcher dispatcher;
```

#### For API classes
```cpp
// Old
discord::HTTPClient client(token);
discord::RestEndpoints endpoints;

// New (explicit but optional)
discord::api::HTTPClient client(token);
discord::api::RestEndpoints endpoints;
```

### Step 4: Update Build Commands

```bash
# No changes needed if using CMake properly configured
# Just rebuild with the new project structure

cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Code Examples

### Example 1: Simple Bot

```cpp
// Old way
#include "discord/client.h"
#include "discord/api/http_client.h"

// New way
#include <discord.h>

int main() {
    const char* token = std::getenv("DISCORD_BOT_TOKEN");
    
    // Both old and new code work identically
    discord::DiscordClient client(token);
    
    client.on_ready([]() {
        std::cout << "Ready!" << std::endl;
    });
    
    client.on_message([](const nlohmann::json& msg) {
        std::cout << "Message: " << msg["content"] << std::endl;
    });
    
    client.run();
    return 0;
}
```

### Example 2: Advanced Usage with Explicit Namespaces

```cpp
// Old way (would need multiple includes)
#include "discord/client.h"
#include "discord/api/rate_limiter.h"
#include "discord/cache/memory_cache.h"

// New way
#include <discord.h>

int main() {
    using namespace discord;
    
    // Create client
    auto client = std::make_unique<core::DiscordClient>(token);
    
    // Setup cache
    auto cache = std::make_shared<cache::MemoryCache>();
    client->set_cache(cache);
    
    // Setup rate limiter
    auto rate_limiter = std::make_shared<api::RateLimiter>();
    client->set_rate_limiter(rate_limiter);
    
    // Setup logging
    auto logger = std::make_shared<utils::Logger>();
    logger->set_level(utils::ILogger::Level::Debug);
    client->set_logger(logger);
    
    // Setup commands
    auto& command_manager = client->get_command_manager();
    command_manager.add_command("help", [](const nlohmann::json& msg) {
        // Handle command
    });
    
    client->run();
    return 0;
}
```

### Example 3: Using Specific Modules

```cpp
// Old way
#include "discord/api/http_client.h"
#include "discord/api/rate_limiter.h"

// New way
#include <discord/api.h>  // Get everything API-related

using namespace discord::api;

int main() {
    HTTPClient http(token);
    RateLimiter limiter;
    
    // Use API directly
    auto result = http.get("/users/@me");
    
    return 0;
}
```

## Backward Compatibility

### Good News ✅
- Old include paths **still work** (mostly)
- Old namespace references **still work**
- Existing code **doesn't need changes**
- You can migrate **gradually**

### What Changed ⚠️
- New modular headers provide cleaner organization
- Sub-namespaces are now available (optional)
- Some includes may need paths adjusted
- CMakeLists.txt target name updated (discord → discord_cpp)

## Common Issues and Solutions

### Issue 1: "Cannot find discord.h"

**Solution 1: Using CMake properly**
```cmake
find_package(discord_cpp REQUIRED)
target_include_directories(myapp PUBLIC ${discord_cpp_INCLUDE_DIRS})
```

**Solution 2: Manual include path**
```cpp
#include "discord/discord.h"
// Or use relative path based on your build system
```

### Issue 2: "discord::DiscordClient not found"

**Solution**
```cpp
// Make sure you include the right header
#include <discord.h>
// or
#include <discord/core.h>
// or
#include <discord/core/client.h>
```

### Issue 3: "APIEndpoints namespace not found"

**Before**
```cpp
discord::APIEndpoints::get_user(user_id);
```

**After**
```cpp
// Option 1: Use the api namespace
discord::api::RestEndpoints::get_user(user_id);

// Option 2: Add using namespace
using namespace discord::api;
RestEndpoints::get_user(user_id);
```

### Issue 4: Build errors with CURL/OpenSSL

**Solution: Ensure dependencies are installed**
```bash
# Ubuntu/Debian
sudo apt-get install libcurl4-openssl-dev libssl-dev

# Or in CMakeLists.txt, ensure proper linking
target_link_libraries(discord_cpp PRIVATE ${CURL_LIBRARIES} OpenSSL::SSL)
```

## Testing Your Migration

### Quick Test Script

```cpp
#include <discord.h>
#include <iostream>

int main() {
    std::cout << "discord.cpp version: " << discord::version() << std::endl;
    std::cout << "Build info: " << discord::build_info() << std::endl;
    
    // Test that classes are accessible
    try {
        // This will fail without a token, but shows compilation works
        discord::core::DiscordClient client("test");
        std::cout << "✓ Core module accessible" << std::endl;
    } catch (...) { }
    
    return 0;
}
```

### Build and Verify

```bash
# Build the project
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Try building your old code
g++ -std=c++23 -I/path/to/discord.cpp/include old_code.cpp \
    -L/path/to/discord.cpp/build/lib -ldiscord_cpp \
    -o old_code

# Should compile without errors
```

## Gradual Migration Strategy

If you have a large codebase, you don't need to change everything at once:

### Phase 1: Update Build System (Week 1)
- Update CMakeLists.txt
- Change target name to discord_cpp
- Ensure all dependencies are linked

### Phase 2: Update Includes (Week 2)
- Replace multiple specific includes with `#include <discord.h>`
- Verify compilation

### Phase 3: Use New Namespaces (Week 3)
- Optionally update code to use explicit namespaces
- Add subnamespace qualifiers where beneficial
- Update documentation

### Phase 4: Optimize and Test (Week 4)
- Profile to find bottlenecks
- Add unit tests if not present
- Verify all functionality works

## Rollback Plan

If you encounter issues:

1. **Keep old include paths available**
   - The new headers re-export old symbols
   - Old code should still work

2. **Use version pinning**
   - Keep using the old version until ready
   - Don't rush the migration

3. **Test thoroughly**
   - Build in isolation first
   - Test with your actual bot code
   - Verify all features work

## Questions?

Refer to:
- **Architecture** → `ARCHITECTURE.md`
- **Module Guide** → `MODULAR_ARCHITECTURE.md`
- **Quick Start** → `QUICKSTART.md`
- **Development** → `DEVELOPMENT.md`

## Checklist

Before considering migration complete:

- [ ] All includes updated to new headers
- [ ] CMakeLists.txt uses new target names
- [ ] Code compiles without warnings
- [ ] All features work as before
- [ ] Unit tests pass (if you have them)
- [ ] Example code runs successfully
- [ ] Documentation updated for your team
- [ ] Performance is acceptable
- [ ] No functionality regressions

## Support

For issues during migration:

1. Check the relevant documentation
2. Review example code in `examples/`
3. Verify dependencies are installed
4. Check compilation flags match C++23 requirements
5. Review error messages carefully

The migration should be smooth since the new structure maintains backward compatibility while providing better organization!
