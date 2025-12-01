# Quick Start Guide

## Installation

### Prerequisites
- CMake 3.20 or later
- C++23 compatible compiler
- nlohmann_json library
- OpenSSL library
- libcurl library

### Building
```bash
git clone <repository>
cd discord.cpp
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build .
cmake --install . --prefix /usr/local
```

## Basic Usage

### Simplest Example
```cpp
#include <discord.h>
#include <iostream>

int main() {
    const char* token = std::getenv("DISCORD_BOT_TOKEN");
    if (!token) {
        std::cerr << "Set DISCORD_BOT_TOKEN environment variable" << std::endl;
        return 1;
    }
    
    discord::DiscordClient client(token);
    
    // Ready event
    client.on_ready([]() {
        std::cout << "Bot is ready!" << std::endl;
    });
    
    // Message event
    client.on_message([](const nlohmann::json& msg) {
        std::cout << "Message: " << msg["content"] << std::endl;
    });
    
    client.run();
    return 0;
}
```

## Module Selection

### Core Only
```cpp
#include <discord/core.h>

discord::DiscordClient client(token);
```

### Core + Events
```cpp
#include <discord/core.h>
#include <discord/events.h>

client.on_message([](const nlohmann::json& msg) { });
```

### Full API Access
```cpp
#include <discord.h>

// Access all modules
discord::DiscordClient client(token);
discord::CacheManager cache;
discord::HTTPClient http(token);
discord::ThreadPool thread_pool;
```

## Common Tasks

### Sending Messages
```cpp
client.on_message([&client](const nlohmann::json& msg) {
    if (msg["content"] == "!ping") {
        client.send_message(msg["channel_id"], "Pong! 🏓");
    }
});
```

### Handling Commands
```cpp
client.add_command("hello", [&client](const nlohmann::json& msg) {
    client.send_message(msg["channel_id"], "Hello! 👋");
});
```

### Accessing User Info
```cpp
client.on_ready([&client]() {
    auto user = client.get_user("USER_ID");
    std::cout << "User: " << user["username"] << std::endl;
});
```

### Creating Embeds
```cpp
discord::utils::EmbedBuilder embed;
embed.set_title("My Embed")
     .set_description("This is an example embed")
     .set_color(0xFF5733);

nlohmann::json embed_json = embed.build();
client.send_message(channel_id, embed_json);
```

## Environment Setup

### Linux/macOS
```bash
# Install dependencies (Ubuntu/Debian)
sudo apt-get install nlohmann-json3-dev libssl-dev libcurl4-openssl-dev

# Or with Homebrew (macOS)
brew install nlohmann-json openssl curl websocketpp
```

### Windows
```powershell
# Using vcpkg
vcpkg install nlohmann-json openssl curl websocketpp

# Then configure CMake to use vcpkg toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE="C:\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

## Build Options

```bash
# Enable examples
cmake .. -DBUILD_EXAMPLES=ON

# Enable tests
cmake .. -DBUILD_TESTS=ON

# Build as shared library
cmake .. -DBUILD_SHARED_LIBS=ON

# Debug build with symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug
```

## Troubleshooting

### "Could not find nlohmann_json"
```bash
# Install the library
sudo apt-get install nlohmann-json3-dev

# Or provide path to CMake
cmake .. -Dnlohmann_json_DIR=/path/to/nlohmann_json
```

### C++23 Compiler Error
```bash
# Use a newer compiler
export CXX=g++-13  # or clang-17, MSVC 2022

# Or use C++20 instead
cmake .. -DCMAKE_CXX_STANDARD=20
```

### WebSocket Connection Failed
- Ensure OpenSSL certificates are installed
- Check firewall settings for outbound connections
- Verify Discord API endpoint accessibility

### Rate Limiting Issues
- Respect Discord's rate limits (check response headers)
- Use built-in rate limiter properly
- Implement exponential backoff

## Documentation Links

- [Discord Developer Portal](https://discord.com/developers)
- [Discord API Reference](https://discord.com/developers/docs)
- [discord.cpp Architecture](./MODULAR_ARCHITECTURE.md)
- [Development Guide](./DEVELOPMENT.md)
- [Coding Standards](./CODING_STANDARDS.md)

## Next Steps

1. **Explore Examples** - Check `examples/` directory
2. **Read the Docs** - Review `MODULAR_ARCHITECTURE.md`
3. **Try Examples** - Build and run example programs
4. **Integrate** - Use discord.cpp in your project
5. **Contribute** - Submit improvements and features

## Getting Help

### Check These First
1. Discord API documentation
2. Example code in `examples/`
3. Module documentation in `MODULAR_ARCHITECTURE.md`
4. Development guide in `DEVELOPMENT.md`

### Community Resources
- Discord Developer Community
- Discord.py documentation (similar concepts)
- C++ reference: cppreference.com

## Important Notes

⚠️ **Token Security**
- Never commit your bot token to version control
- Use environment variables to pass tokens
- Regenerate tokens if accidentally exposed
- Rotate tokens regularly

⚠️ **Rate Limits**
- Discord enforces strict rate limits
- Implement proper backoff strategies
- Cache data to minimize requests
- Use bulk operations when available

⚠️ **Intents**
- Specify required Gateway Intents
- Not all events available without intents
- Privileged intents require approval
- Some intents are restricted

## Example Project Structure

```
my_bot/
├── CMakeLists.txt
├── src/
│   ├── main.cpp
│   ├── commands.cpp
│   └── handlers.cpp
├── include/
│   ├── commands.h
│   └── handlers.h
└── .env.example
```

## CMakeLists.txt Template

```cmake
cmake_minimum_required(VERSION 3.20)
project(my_bot)

set(CMAKE_CXX_STANDARD 23)

find_package(discord_cpp REQUIRED)
find_package(nlohmann_json REQUIRED)

add_executable(my_bot 
    src/main.cpp
    src/commands.cpp
    src/handlers.cpp
)

target_link_libraries(my_bot
    discord_cpp::discord_cpp
    nlohmann_json::nlohmann_json
)

target_include_directories(my_bot PRIVATE include)
```

## Tips & Tricks

### Using Multiple Handlers
```cpp
auto handler1 = [](const nlohmann::json& msg) {
    // Handle something
};

auto handler2 = [](const nlohmann::json& msg) {
    // Handle something else
};

client.on_message(handler1);
client.on_message(handler2);  // Multiple handlers supported
```

### Async Operations
```cpp
// All I/O operations return futures
auto user_future = client.get_user_async(user_id);

// Wait for result
auto user = user_future.get();

// Or process asynchronously
user_future.then([](const auto& user) {
    std::cout << user["username"] << std::endl;
});
```

### Custom Logging
```cpp
auto logger = std::make_shared<discord::Logger>();
logger->set_level(discord::ILogger::Level::Debug);
client.set_logger(logger);
```

### Caching Data
```cpp
auto cache = std::make_shared<discord::MemoryCache>();
client.set_cache(cache);

// Or use Redis for distributed caching
auto redis_cache = std::make_shared<discord::RedisCache>("localhost:6379");
client.set_cache(redis_cache);
```

Happy coding! 🚀
