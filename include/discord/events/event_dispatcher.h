#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <shared_mutex>
#include <chrono>
#include <variant>
#include <nlohmann/json.hpp>
#include "../core/interfaces.h"

namespace discord {

/**
 * @brief Event filter function type
 */
using EventFilter = std::function<bool(const nlohmann::json&)>;

/**
 * @brief Event handler function type
 */
using EventCallback = std::function<void(const nlohmann::json&)>;

/**
 * @brief Event handler with priority
 */
struct EventHandlerInfo {
    EventCallback callback;
    int priority;
    std::string id;
    bool once;
    std::chrono::steady_clock::time_point created_at;
    
    EventHandlerInfo(EventCallback cb, int prio = 0, const std::string& handler_id = "", bool is_once = false)
        : callback(std::move(cb)), priority(prio), id(handler_id), once(is_once)
        , created_at(std::chrono::steady_clock::now()) {}
};

/**
 * @brief Event collector configuration
 */
struct CollectorConfig {
    std::chrono::milliseconds timeout;
    int max_matches;
    bool dispose_on_timeout;
    
    CollectorConfig(std::chrono::milliseconds to = std::chrono::milliseconds(30000), 
                   int max = 1, bool dispose = true)
        : timeout(to), max_matches(max), dispose_on_timeout(dispose) {}
};

/**
 * @brief Event collector for waiting for specific events
 */
template<typename T>
class EventCollector {
public:
    using FilterFunction = std::function<bool(const T&)>;
    using CollectorCallback = std::function<void(const T&)>;
    
private:
    std::vector<T> collected_items_;
    FilterFunction filter_;
    CollectorConfig config_;
    std::string handler_id_;
    bool is_active_;
    std::chrono::steady_clock::time_point start_time_;
    mutable std::shared_mutex mutex_;
    
    /**
     * @brief Check if collector should stop
     * @return True if max matches reached
     */
    bool should_stop_collecting() const {
        return config_.max_matches > 0 && 
               collected_items_.size() >= static_cast<size_t>(config_.max_matches);
    }
    
    /**
     * @brief Check if collector has timed out
     * @return True if timeout exceeded
     */
    bool is_timed_out() const {
        if (config_.timeout.count() == 0) {
            return false;
        }
        
        auto now = std::chrono::steady_clock::now();
        return (now - start_time_) > config_.timeout;
    }

public:
    /**
     * @brief Construct EventCollector
     * @param filter Filter function for events
     * @param config Collector configuration
     */
    explicit EventCollector(FilterFunction filter = nullptr, 
                        const CollectorConfig& config = CollectorConfig{})
        : filter_(std::move(filter)), config_(config), is_active_(false) {}
    
    /**
     * @brief Destructor - cleans up handler
     */
    ~EventCollector() {
        stop();
    }
    
    /**
     * @brief Start collecting events
     * @param dispatcher Event dispatcher to register with
     * @param event_name Event name to listen for
     * @return Handler ID for cleanup
     */
    std::string start(class EventDispatcher* dispatcher, const std::string& event_name);
    
    /**
     * @brief Stop collecting events
     */
    void stop();
    
    /**
     * @brief Process incoming event
     * @param event Event to process
     * @return True if event was collected
     */
    bool process_event(const nlohmann::json& event);
    
    /**
     * @brief Get collected items
     * @return Vector of collected items
     */
    std::vector<T> get_collected() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return collected_items_;
    }
    
    /**
     * @brief Get first collected item
     * @return First item or empty optional
     */
    std::optional<T> first() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return collected_items_.empty() ? std::nullopt : std::optional<T>(collected_items_.front());
    }
    
    /**
     * @brief Wait for first match
     * @param timeout Maximum time to wait
     * @return First matching item or empty optional
     */
    std::optional<T> wait_for_first(std::chrono::milliseconds timeout = std::chrono::milliseconds(30000));
    
    /**
     * @brief Wait for all matches
     * @param timeout Maximum time to wait
     * @return All matching items or empty vector
     */
    std::vector<T> wait_for_all(std::chrono::milliseconds timeout = std::chrono::milliseconds(30000));
    
    /**
     * @brief Check if collector is active
     * @return True if actively collecting
     */
    bool is_active() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return is_active_;
    }
    
    /**
     * @brief Get number of collected items
     * @return Number of items collected
     */
    size_t count() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return collected_items_.size();
    }
    
    /**
     * @brief Clear collected items
     */
    void clear() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        collected_items_.clear();
    }
};

/**
 * @brief Event middleware interface
 */
class IEventMiddleware {
public:
    virtual ~IEventMiddleware() = default;
    
    /**
     * @brief Process event before handlers
     * @param event_name Event name
     * @param event_data Event data
     * @param next Next middleware in chain
     * @return True if event should continue processing
     */
    virtual bool process(const std::string& event_name, 
                      const nlohmann::json& event_data, 
                      std::function<void()> next) = 0;
    
    /**
     * @brief Get middleware priority
     * @return Priority value (higher = earlier execution)
     */
    virtual int get_priority() const { return 0; }
    
    /**
     * @brief Get middleware name
     * @return Middleware identifier
     */
    virtual std::string get_name() const = 0;
};

/**
 * @brief Comprehensive Event Dispatcher for Discord.cpp
 * 
 * Provides advanced event handling with filters, collectors, middleware,
 * priority ordering, and performance monitoring.
 * 
 * @todo TODO.md: Implement comprehensive event system with collectors and filters
 */
class EventDispatcher {
public:
    using EventMap = std::unordered_map<std::string, std::vector<EventHandlerInfo>>;
    using MiddlewareList = std::vector<std::shared_ptr<IEventMiddleware>>;

private:
    EventMap handlers_;
    MiddlewareList middleware_;
    mutable std::shared_mutex handlers_mutex_;
    mutable std::shared_mutex middleware_mutex_;
    
    // Performance tracking
    std::atomic<uint64_t> events_dispatched_{0};
    std::atomic<uint64_t> handlers_executed_{0};
    std::chrono::steady_clock::time_point start_time_;
    
    // Active collectors tracking
    std::unordered_map<std::string, std::shared_ptr<void>> active_collectors_;
    mutable std::shared_mutex collectors_mutex_;

    /**
     * @brief Sort handlers by priority
     * @param handlers Vector of handlers to sort
     */
    void sort_handlers(std::vector<EventHandlerInfo>& handlers) const;

    /**
     * @brief Execute middleware chain
     * @param event_name Event name
     * @param event_data Event data
     * @param final_handler Final handler to execute
     */
    void execute_middleware_chain(const std::string& event_name,
                               const nlohmann::json& event_data,
                               std::function<void()> final_handler);

    /**
     * @brief Update performance statistics
     */
    void update_stats();

public:
    /**
     * @brief Construct EventDispatcher
     */
    EventDispatcher();

    /**
     * @brief Destructor - cleans up all handlers
     */
    ~EventDispatcher();

    /**
     * @brief Register event handler
     * @param event_name Event name to listen for
     * @param callback Handler function
     * @param priority Handler priority (higher = earlier)
     * @param handler_id Unique handler identifier
     * @param once Whether to remove after first execution
     * @return Handler ID for removal
     */
    std::string on(const std::string& event_name, 
                   EventCallback callback,
                   int priority = 0,
                   const std::string& handler_id = "",
                   bool once = false);

    /**
     * @brief Remove event handler
     * @param event_name Event name
     * @param handler_id Handler ID to remove
     * @return True if handler was removed
     */
    bool off(const std::string& event_name, const std::string& handler_id);

    /**
     * @brief Remove all handlers for event
     * @param event_name Event name
     * @return Number of handlers removed
     */
    size_t off_all(const std::string& event_name);

    /**
     * @brief Emit event to all handlers
     * @param event_name Event name
     * @param event_data Event data
     */
    void emit(const std::string& event_name, const nlohmann::json& event_data);

    /**
     * @brief Emit event with filters
     * @param event_name Event name
     * @param event_data Event data
     * @param filters Vector of filter functions
     */
    void emit_filtered(const std::string& event_name,
                     const nlohmann::json& event_data,
                     const std::vector<EventFilter>& filters);

    /**
     * @brief Wait for specific event
     * @param event_name Event name to wait for
     * @param filter Filter function
     * @param timeout Maximum wait time
     * @return Matching event data or empty optional
     */
    std::optional<nlohmann::json> wait_for(const std::string& event_name,
                                           EventFilter filter = nullptr,
                                           std::chrono::milliseconds timeout = std::chrono::milliseconds(30000));

    /**
     * @brief Create event collector
     * @tparam T Type to collect
     * @param event_name Event name to collect
     * @param filter Filter function
     * @param config Collector configuration
     * @return Shared pointer to collector
     */
    template<typename T>
    std::shared_ptr<EventCollector<T>> create_collector(const std::string& event_name,
                                                    typename EventCollector<T>::FilterFunction filter = nullptr,
                                                    const CollectorConfig& config = CollectorConfig{});

    /**
     * @brief Add middleware
     * @param middleware Middleware to add
     */
    void add_middleware(std::shared_ptr<IEventMiddleware> middleware);

    /**
     * @brief Remove middleware
     * @param middleware_name Name of middleware to remove
     * @return True if middleware was removed
     */
    bool remove_middleware(const std::string& middleware_name);

    /**
     * @brief Get all handlers for event
     * @param event_name Event name
     * @return Vector of handler information
     */
    std::vector<EventHandlerInfo> get_handlers(const std::string& event_name) const;

    /**
     * @brief Get performance statistics
     * @return JSON object with statistics
     */
    nlohmann::json get_statistics() const;

    /**
     * @brief Reset performance statistics
     */
    void reset_statistics();

    /**
     * @brief Handle Discord gateway dispatch
     * @param payload Gateway payload
     */
    void handle_dispatch(const nlohmann::json& payload);

    /**
     * @brief Get number of registered handlers
     * @return Total handler count
     */
    size_t get_handler_count() const;

    /**
     * @brief Get number of active collectors
     * @return Active collector count
     */
    size_t get_active_collector_count() const;

    /**
     * @brief Clear all handlers and collectors
     */
    void clear();
};

/**
 * @brief Built-in event filters
 */
namespace EventFilters {
    /**
     * @brief Filter by user ID
     * @param user_id User ID to match
     * @return Filter function
     */
    EventFilter by_user_id(const std::string& user_id);
    
    /**
     * @brief Filter by channel ID
     * @param channel_id Channel ID to match
     * @return Filter function
     */
    EventFilter by_channel_id(const std::string& channel_id);
    
    /**
     * @brief Filter by guild ID
     * @param guild_id Guild ID to match
     * @return Filter function
     */
    EventFilter by_guild_id(const std::string& guild_id);
    
    /**
     * @brief Filter by message content
     * @param content Content to match (supports wildcards)
     * @return Filter function
     */
    EventFilter by_content(const std::string& content);
    
    /**
     * @brief Filter by author bot status
     * @param is_bot Whether author should be a bot
     * @return Filter function
     */
    EventFilter by_bot(bool is_bot);
    
    /**
     * @brief Combine multiple filters with AND logic
     * @param filters Vector of filters to combine
     * @return Combined filter function
     */
    EventFilter and_filter(const std::vector<EventFilter>& filters);
    
    /**
     * @brief Combine multiple filters with OR logic
     * @param filters Vector of filters to combine
     * @return Combined filter function
     */
    EventFilter or_filter(const std::vector<EventFilter>& filters);
    
    /**
     * @brief Negate a filter
     * @param filter Filter to negate
     * @return Negated filter function
     */
    EventFilter not_filter(EventFilter filter);
}

/**
 * @brief Built-in middleware implementations
 */
namespace EventMiddleware {
    /**
     * @brief Rate limiting middleware
     */
    class RateLimiter : public IEventMiddleware {
    private:
        std::unordered_map<std::string, std::pair<int, std::chrono::steady_clock::time_point>> counters_;
        int max_events_per_window_;
        std::chrono::milliseconds window_size_;
        mutable std::shared_mutex mutex_;
        
    public:
        RateLimiter(int max_events = 100, std::chrono::milliseconds window = std::chrono::minutes(1));
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return 100; }
        std::string get_name() const override { return "RateLimiter"; }
    };
    
    /**
     * @brief Logging middleware
     */
    class Logger : public IEventMiddleware {
    private:
        std::vector<std::string> logged_events_;
        bool log_all_events_;
        
    public:
        explicit Logger(bool log_all = false);
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return -100; }
        std::string get_name() const override { return "Logger"; }
    };
    
    /**
     * @brief Event validation middleware
     */
    class Validator : public IEventMiddleware {
    private:
        std::unordered_map<std::string, std::function<bool(const nlohmann::json&)>> validators_;
        
    public:
        void add_validator(const std::string& event_name, std::function<bool(const nlohmann::json&)> validator);
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return 50; }
        std::string get_name() const override { return "Validator"; }
    };
}

} // namespace discord