# Coding Standards

## Overview

This document defines the coding standards for discord.cpp. All contributors should follow these guidelines to maintain consistency and code quality.

## Code Style

### Naming Conventions

#### Classes and Structs
```cpp
class HTTPClient { };          // PascalCase
struct EventContext { };        // PascalCase
```

#### Functions and Methods
```cpp
void send_message();           // snake_case
bool is_connected();           // snake_case for getters
void set_timeout();            // snake_case for setters
```

#### Variables
```cpp
std::string channel_id;        // snake_case
int message_count = 0;         // snake_case
```

#### Constants
```cpp
constexpr int API_VERSION = 10;         // UPPER_SNAKE_CASE
static constexpr auto TIMEOUT_MS = 30s; // UPPER_SNAKE_CASE
```

#### Enums
```cpp
enum class GatewayOpcode {     // PascalCase for enum name
    DISPATCH = 0,              // UPPER_SNAKE_CASE for values
    HEARTBEAT = 1
};
```

#### Namespaces
```cpp
namespace discord::api { }     // lowercase with ::
namespace discord::utils { }   // lowercase
```

### Formatting

#### Indentation and Braces
```cpp
// Use 4 spaces or 1 tab per indentation level
namespace discord {
    class MyClass {
    public:
        void my_method() {
            if (condition) {
                do_something();
            }
        }
    };
}
```

#### Line Length
- Aim for 100 characters maximum
- Break long lines at logical points
- Continuation lines should be indented appropriately

#### Spacing
```cpp
// Space around operators
int result = a + b;
if (x == y) { }
for (int i = 0; i < 10; ++i) { }

// Space after keywords
if (condition) { }
while (condition) { }
for (auto& item : collection) { }

// No space before function call parentheses
function_name();
not function_name ();

// Space in template parameters
std::vector<std::string> vec;
template<typename T>
class MyTemplate { };
```

### Header Organization

```cpp
#pragma once

// 1. Standard library headers
#include <string>
#include <vector>
#include <memory>

// 2. Third-party headers
#include <nlohmann/json.hpp>
#include <openssl/ssl.h>

// 3. Project headers
#include "../core/interfaces.h"
#include "../utils/types.h"

namespace discord {

// Forward declarations
class DiscordClient;
struct User;

// Type aliases
using UserList = std::vector<User>;

// Constants
static constexpr int DEFAULT_TIMEOUT = 30000;

// Class definition
class MyClass {
    // ...
};

} // namespace discord
```

## C++ Best Practices

### Memory Management

Use smart pointers instead of raw pointers:
```cpp
// Good
std::unique_ptr<HTTPClient> client;
std::shared_ptr<Cache> cache;

// Avoid
HTTPClient* client = new HTTPClient();
delete client;
```

### Error Handling

Use the Result type for fallible operations:
```cpp
Result<nlohmann::json> get_user(const std::string& id) {
    try {
        // Perform operation
        return nlohmann::json::parse(response);
    } catch (const std::exception& e) {
        return Result<nlohmann::json>(std::string("Error: ") + e.what());
    }
}
```

Or use exceptions for exceptional cases:
```cpp
void connect() {
    if (!validate_token(token_)) {
        throw AuthenticationException("Invalid token");
    }
    // Connect...
}
```

### Resource Acquisition Is Initialization (RAII)

```cpp
class HTTPClient {
private:
    CURL* curl_;
    
public:
    HTTPClient() {
        curl_global_init(CURL_GLOBAL_DEFAULT);
        curl_ = curl_easy_init();
    }
    
    ~HTTPClient() {
        if (curl_) {
            curl_easy_cleanup(curl_);
        }
        curl_global_cleanup();
    }
    
    // Prevent copying
    HTTPClient(const HTTPClient&) = delete;
    HTTPClient& operator=(const HTTPClient&) = delete;
};
```

### Const Correctness

```cpp
// Const for methods that don't modify state
bool is_connected() const;
const std::string& get_token() const;

// Const references to avoid copying
void process_message(const nlohmann::json& message);

// Const for variables that shouldn't change
const int MAX_ATTEMPTS = 3;
```

### Move Semantics

```cpp
class MyClass {
private:
    std::vector<std::string> data_;
    
public:
    // Move constructor
    MyClass(MyClass&& other) noexcept 
        : data_(std::move(other.data_)) { }
    
    // Move assignment
    MyClass& operator=(MyClass&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
        }
        return *this;
    }
    
    // Return by move
    std::vector<std::string> get_data() && {
        return std::move(data_);
    }
};
```

### Async and Threading

```cpp
// Use futures for async operations
std::future<nlohmann::json> get_user_async(const std::string& id) {
    return std::async(std::launch::async, [this, id]() {
        return get_user(id);
    });
}

// Use thread pool for long-running tasks
thread_pool_.submit([]() {
    // Long-running task
});
```

## Documentation

### Comment Style

Use C++ style comments:
```cpp
// Single-line comment

// Multi-line comment
// spanning multiple lines
// for clarity
```

### Function Documentation

```cpp
/**
 * @brief Send a message to a Discord channel
 * 
 * @param channel_id The ID of the target channel
 * @param content The message content to send
 * 
 * @return Result containing the sent message or error
 * 
 * @throws AuthenticationException if not authenticated
 * @throws RateLimitException if rate limited
 * 
 * @note Message content is limited to 2000 characters
 * 
 * @see https://discord.com/developers/docs/resources/channel#create-message
 */
Result<nlohmann::json> send_message(
    const std::string& channel_id,
    const std::string& content
);
```

### Class Documentation

```cpp
/**
 * @brief HTTP client for Discord API communication
 * 
 * Manages HTTP requests to Discord's REST API with automatic
 * rate limiting and retry logic.
 * 
 * @note Thread-safe for concurrent requests
 * 
 * @example
 * HTTPClient client("bot_token");
 * auto response = client.get("/users/@me");
 */
class HTTPClient {
    // ...
};
```

## Testing

### Test Organization

```cpp
// tests/test_http_client.cpp
#include <catch2/catch.hpp>
#include "../include/discord/api/http_client.h"

namespace discord::tests {

TEST_CASE("HTTPClient::send_request") {
    SECTION("successful request") {
        // Test successful case
    }
    
    SECTION("timeout handling") {
        // Test timeout case
    }
    
    SECTION("rate limiting") {
        // Test rate limit case
    }
}

} // namespace discord::tests
```

### Test Naming

- Test files: `test_*.cpp` or `*_test.cpp`
- Test cases: Describe what is being tested
- Assertions: One logical assertion per test

## Performance Considerations

### Avoid Unnecessary Copies

```cpp
// Bad - creates copies
void process(std::vector<std::string> items) {
    for (auto item : items) { }
}

// Good - uses references
void process(const std::vector<std::string>& items) {
    for (const auto& item : items) { }
}
```

### Use move semantics for large objects

```cpp
// Bad
std::string get_data() {
    std::string result = "...";
    return result;  // Copy
}

// Good - compiler should elide copy, but move semantics help
std::string get_data() {
    std::string result = "...";
    return result;  // Move or RVO
}
```

### Cache frequently used computations

```cpp
class Gateway {
private:
    mutable std::optional<std::string> cached_gateway_url_;
    mutable std::chrono::system_clock::time_point cache_time_;
    
public:
    const std::string& get_gateway_url() const {
        auto now = std::chrono::system_clock::now();
        if (!cached_gateway_url_ || 
            (now - cache_time_) > std::chrono::hours(1)) {
            cached_gateway_url_ = fetch_gateway_url();
            cache_time_ = now;
        }
        return *cached_gateway_url_;
    }
};
```

## Security

### Input Validation

```cpp
void set_token(const std::string& token) {
    if (token.empty() || token.length() < 50) {
        throw ValidationException("Invalid token format");
    }
    // Sanitize token
    token_ = token;
}
```

### Secure Storage

- Never log tokens or secrets
- Clear sensitive data after use
- Use secure string classes if available
- Protect against timing attacks in comparisons

## Logging

Use the Logger interface for consistent logging:

```cpp
logger_->log(ILogger::Level::Info, "Bot started");
logger_->log(ILogger::Level::Warning, "Rate limit approaching");
logger_->log(ILogger::Level::Error, "Failed to connect to gateway");
```

## Code Review Checklist

- [ ] Follows naming conventions
- [ ] Proper use of const
- [ ] Proper use of references
- [ ] Smart pointers used instead of raw pointers
- [ ] RAII principles followed
- [ ] Thread-safety considered
- [ ] Error handling appropriate
- [ ] Performance acceptable
- [ ] Security concerns addressed
- [ ] Code is documented
- [ ] Tests included for new features
- [ ] No compiler warnings

## Useful Tools

- **clang-format**: For automatic formatting
- **clang-tidy**: For static analysis
- **valgrind**: For memory leak detection
- **perf**: For performance profiling
- **gdb/lldb**: For debugging

Example `.clang-format`:
```yaml
BasedOnStyle: LLVM
ColumnLimit: 100
IndentWidth: 4
UseTab: Never
Standard: c++23
```
