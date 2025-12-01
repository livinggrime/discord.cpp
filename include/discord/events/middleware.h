#pragma once

#include <string>
#include <functional>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "event_dispatcher.h"

namespace discord {

/**
 * @brief Event middleware for processing events before handlers
 * 
 * Provides a chain of middleware that can modify, validate,
 * or block events before they reach the main handlers.
 * 
 * @todo TODO.md: Implement comprehensive event system with collectors and filters
 */
class EventMiddleware {
public:
    /**
     * @brief Middleware processing function
     * @param event_name Event name
     * @param event_data Event data
     * @param next Next middleware in chain
     * @return True if event should continue processing
     */
    using MiddlewareFunction = std::function<bool(const std::string&,
                                                   const nlohmann::json&,
                                                   std::function<void()>)>;

private:
    std::vector<std::shared_ptr<IEventMiddleware>> middleware_;
    mutable std::shared_mutex mutex_;

    /**
     * @brief Sort middleware by priority
     */
    void sort_middleware();

public:
    /**
     * @brief Construct EventMiddleware
     */
    EventMiddleware();

    /**
     * @brief Destructor
     */
    ~EventMiddleware();

    /**
     * @brief Add middleware to chain
     * @param middleware Middleware to add
     */
    void add_middleware(std::shared_ptr<IEventMiddleware> middleware);

    /**
     * @brief Remove middleware from chain
     * @param middleware_name Name of middleware to remove
     * @return True if middleware was removed
     */
    bool remove_middleware(const std::string& middleware_name);

    /**
     * @brief Process event through middleware chain
     * @param event_name Event name
     * @param event_data Event data
     * @param final_handler Final handler to execute
     */
    void process_event(const std::string& event_name,
                     const nlohmann::json& event_data,
                     std::function<void()> final_handler);

    /**
     * @brief Get all middleware
     * @return Vector of middleware
     */
    std::vector<std::shared_ptr<IEventMiddleware>> get_middleware() const;

    /**
     * @brief Clear all middleware
     */
    void clear();
};

/**
 * @brief Built-in middleware implementations
 */
namespace BuiltInMiddleware {

    /**
     * @brief Authentication middleware
     * 
     * Validates that events contain proper authentication data
     * and filters out unauthorized events.
     */
    class Authentication : public IEventMiddleware {
    private:
        std::string bot_token_;
        bool require_user_id_;
        
    public:
        explicit Authentication(const std::string& token, bool require_user_id = false);
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return 90; }
        std::string get_name() const override { return "Authentication"; }
    };

    /**
     * @brief Permission checking middleware
     * 
     * Validates that events have required permissions
     * and filters out unauthorized actions.
     */
    class PermissionChecker : public IEventMiddleware {
    private:
        std::unordered_map<std::string, uint64_t> required_permissions_;
        
    public:
        explicit PermissionChecker(const std::unordered_map<std::string, uint64_t>& permissions);
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return 80; }
        std::string get_name() const override { return "PermissionChecker"; }
    };

    /**
     * @brief Event transformation middleware
     * 
     * Transforms event data before it reaches handlers.
     * Useful for data normalization or enrichment.
     */
    class Transformer : public IEventMiddleware {
    private:
        std::unordered_map<std::string, std::function<nlohmann::json(nlohmann::json)>> transformers_;
        
    public:
        explicit Transformer(const std::unordered_map<std::string, 
                                             std::function<nlohmann::json(nlohmann::json)>>& transformers);
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return 60; }
        std::string get_name() const override { return "Transformer"; }
    };

    /**
     * @brief Event filtering middleware
     * 
     * Filters events based on configurable criteria.
     * Can filter by user, channel, guild, content, etc.
     */
    class Filter : public IEventMiddleware {
    private:
        std::vector<EventFilter> filters_;
        std::string filter_mode_; // "any" or "all"
        
    public:
        explicit Filter(const std::vector<EventFilter>& filters, const std::string& mode = "all");
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return 70; }
        std::string get_name() const override { return "Filter"; }
    };

    /**
     * @brief Event caching middleware
     * 
     * Caches events to prevent duplicate processing
     * and provides event replay capabilities.
     */
    class Cache : public IEventMiddleware {
    private:
        std::unordered_map<std::string, std::vector<nlohmann::json>> event_cache_;
        size_t max_cache_size_;
        std::chrono::milliseconds cache_ttl_;
        mutable std::shared_mutex mutex_;
        
        /**
         * @brief Clean expired cache entries
         */
        void cleanup_cache();
        
    public:
        explicit Cache(size_t max_size = 1000, 
                     std::chrono::milliseconds ttl = std::chrono::minutes(5));
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return 40; }
        std::string get_name() const override { return "Cache"; }
        
        /**
         * @brief Get cached events for event type
         * @param event_name Event name
         * @return Cached events
         */
        std::vector<nlohmann::json> get_cached_events(const std::string& event_name) const;
        
        /**
         * @brief Clear cache for event type
         * @param event_name Event name
         */
        void clear_cache(const std::string& event_name);
    };

    /**
     * @brief Event metrics middleware
     * 
     * Collects metrics about events passing through
     * including counts, timing, and error tracking.
     */
    class Metrics : public IEventMiddleware {
    private:
        std::unordered_map<std::string, uint64_t> event_counts_;
        std::unordered_map<std::string, std::chrono::steady_clock::time_point> last_event_times_;
        std::unordered_map<std::string, uint64_t> error_counts_;
        mutable std::shared_mutex mutex_;
        
    public:
        Metrics() = default;
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return -50; } // Low priority to run last
        std::string get_name() const override { return "Metrics"; }
        
        /**
         * @brief Get event statistics
         * @return JSON object with metrics
         */
        nlohmann::json get_metrics() const;
        
        /**
         * @brief Reset metrics
         */
        void reset_metrics();
    };

    /**
     * @brief Event debugging middleware
     * 
     * Logs detailed information about events for debugging.
     * Can be configured to log only specific event types.
     */
    class Debugger : public IEventMiddleware {
    private:
        std::vector<std::string> debug_events_;
        bool log_all_events_;
        bool include_stack_trace_;
        
    public:
        explicit Debugger(const std::vector<std::string>& events = {}, 
                        bool log_all = false,
                        bool include_stack = false);
        
        bool process(const std::string& event_name,
                   const nlohmann::json& event_data,
                   std::function<void()> next) override;
        
        int get_priority() const override { return -100; } // Very low priority
        std::string get_name() const override { return "Debugger"; }
    };
};

/**
 * @brief Middleware factory for creating common middleware
 */
class MiddlewareFactory {
public:
    /**
     * @brief Create authentication middleware
     * @param token Bot token
     * @param require_user_id Whether to require user ID validation
     * @return Shared pointer to authentication middleware
     */
    static std::shared_ptr<BuiltInMiddleware::Authentication> create_authentication(
        const std::string& token,
        bool require_user_id = false);

    /**
     * @brief Create permission checker middleware
     * @param permissions Required permissions map
     * @return Shared pointer to permission middleware
     */
    static std::shared_ptr<BuiltInMiddleware::PermissionChecker> create_permission_checker(
        const std::unordered_map<std::string, uint64_t>& permissions);

    /**
     * @brief Create transformer middleware
     * @param transformers Event transformation functions
     * @return Shared pointer to transformer middleware
     */
    static std::shared_ptr<BuiltInMiddleware::Transformer> create_transformer(
        const std::unordered_map<std::string, std::function<nlohmann::json(nlohmann::json)>>& transformers);

    /**
     * @brief Create filter middleware
     * @param filters Filter functions
     * @param mode Filter mode ("any" or "all")
     * @return Shared pointer to filter middleware
     */
    static std::shared_ptr<BuiltInMiddleware::Filter> create_filter(
        const std::vector<EventFilter>& filters,
        const std::string& mode = "all");

    /**
     * @brief Create cache middleware
     * @param max_size Maximum cache size
     * @param ttl Cache TTL
     * @return Shared pointer to cache middleware
     */
    static std::shared_ptr<BuiltInMiddleware::Cache> create_cache(
        size_t max_size = 1000,
        std::chrono::milliseconds ttl = std::chrono::minutes(5));

    /**
     * @brief Create metrics middleware
     * @return Shared pointer to metrics middleware
     */
    static std::shared_ptr<BuiltInMiddleware::Metrics> create_metrics();

    /**
     * @brief Create debugger middleware
     * @param events Events to debug
     * @param log_all Whether to log all events
     * @param include_stack Whether to include stack traces
     * @return Shared pointer to debugger middleware
     */
    static std::shared_ptr<BuiltInMiddleware::Debugger> create_debugger(
        const std::vector<std::string>& events = {},
        bool log_all = false,
        bool include_stack = false);
};

} // namespace discord