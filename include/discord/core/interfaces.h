#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <future>
#include <chrono>
#include <optional>
#include <variant>
#include <nlohmann/json.hpp>

namespace discord {

// Forward declarations
class DiscordClient;
struct User;
struct Guild;
struct Channel;

// Core interfaces for extensibility

class ICache {
public:
    virtual ~ICache() = default;
    virtual void set(const std::string& key, const nlohmann::json& value, std::chrono::seconds ttl = {}) = 0;
    virtual std::optional<nlohmann::json> get(const std::string& key) = 0;
    virtual void remove(const std::string& key) = 0;
    virtual void clear() = 0;
    virtual bool exists(const std::string& key) = 0;
    virtual std::vector<std::string> keys(const std::string& pattern = "*") = 0;
};

class IHttpClient {
public:
    using Headers = std::vector<std::pair<std::string, std::string>>;
    using Params = std::vector<std::pair<std::string, std::string>>;
    
    virtual ~IHttpClient() = default;
    virtual std::future<nlohmann::json> get(const std::string& url, const Headers& headers = {}) = 0;
    virtual std::future<nlohmann::json> post(const std::string& url, const nlohmann::json& data, const Headers& headers = {}) = 0;
    virtual std::future<nlohmann::json> put(const std::string& url, const nlohmann::json& data, const Headers& headers = {}) = 0;
    virtual std::future<nlohmann::json> patch(const std::string& url, const nlohmann::json& data, const Headers& headers = {}) = 0;
    virtual std::future<nlohmann::json> delete_(const std::string& url, const Headers& headers = {}) = 0;
    virtual std::future<void> set_timeout(std::chrono::milliseconds timeout) = 0;
};

class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    virtual void handle(const nlohmann::json& event) = 0;
    virtual std::string getEventType() const = 0;
    virtual int getPriority() const { return 0; }
};

class IRateLimiter {
public:
    virtual ~IRateLimiter() = default;
    virtual std::future<bool> can_request(const std::string& bucket) = 0;
    virtual std::future<void> wait_for_slot(const std::string& bucket) = 0;
    virtual void update_limits(const std::string& bucket, int remaining, int reset_after) = 0;
    virtual void reset() = 0;
};

class IWebSocketClient {
public:
    virtual ~IWebSocketClient() = default;
    virtual std::future<bool> connect(const std::string& url) = 0;
    virtual std::future<void> disconnect() = 0;
    virtual std::future<void> send(const nlohmann::json& message) = 0;
    virtual std::future<void> send_heartbeat() = 0;
    virtual void set_message_handler(std::function<void(const nlohmann::json&)> handler) = 0;
    virtual void set_close_handler(std::function<void(int, const std::string&)> handler) = 0;
    virtual bool is_connected() const = 0;
};

class ILogger {
public:
    enum class Level {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warning = 3,
        Error = 4,
        Critical = 5
    };
    
    virtual ~ILogger() = default;
    virtual void log(Level level, const std::string& message) = 0;
    virtual void set_level(Level level) = 0;
    virtual Level get_level() const = 0;
};

class IThreadPool {
public:
    virtual ~IThreadPool() = default;
    virtual std::future<void> submit(std::function<void()> task) = 0;
    virtual void shutdown() = 0;
    virtual size_t get_thread_count() const = 0;
    virtual size_t get_pending_tasks() const = 0;
    
    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        return submit([f = std::forward<F>(f), args...]() { f(args...); });
    }
};

// Event context for handlers
struct EventContext {
    std::string type;
    nlohmann::json data;
    std::chrono::system_clock::time_point timestamp;
    DiscordClient* client;
};



} // namespace discord