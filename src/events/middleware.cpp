#include <discord/events/middleware.h>
#include <discord/utils/logger.h>
#include <algorithm>
#include <chrono>

namespace discord {

// EventMiddleware implementation

EventMiddleware::EventMiddleware() {
    LOG_INFO("EventMiddleware initialized");
}

EventMiddleware::~EventMiddleware() {
    clear();
    LOG_INFO("EventMiddleware destroyed");
}

void EventMiddleware::add_middleware(std::shared_ptr<IEventMiddleware> middleware) {
    if (!middleware) {
        return;
    }
    
    std::unique_lock<std::shared_mutex> lock(mutex_);
    middleware_.push_back(std::move(middleware));
    sort_middleware();
    
    LOG_DEBUG("Added middleware: " + middleware->get_name());
}

bool EventMiddleware::remove_middleware(const std::string& middleware_name) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = std::remove_if(middleware_.begin(), middleware_.end(),
        [&middleware_name](const auto& middleware) {
            return middleware->get_name() == middleware_name;
        });
    
    bool removed = it != middleware_.end();
    if (removed) {
        LOG_DEBUG("Removed middleware: " + middleware_name);
    }
    
    return removed;
}

void EventMiddleware::process_event(const std::string& event_name,
                                 const nlohmann::json& event_data,
                                 std::function<void()> final_handler) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    // Process middleware chain
    size_t index = 0;
    auto next_middleware = [&, final_handler, &index, &event_name, &event_data]() {
        if (index >= middleware_.size()) {
            final_handler();
            return;
        }
        
        auto& middleware = middleware_[index];
        index++;
        
        auto next = [&, final_handler, &index, &event_name, &event_data]() {
            next_middleware();
        };
        
        if (!middleware->process(event_name, event_data, next)) {
            LOG_DEBUG("Middleware " + middleware->get_name() + " blocked event: " + event_name);
        }
    };
    
    // Start middleware chain
    if (middleware_.empty()) {
        final_handler();
    } else {
        next_middleware();
    }
}

std::vector<std::shared_ptr<IEventMiddleware>> EventMiddleware::get_middleware() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    return middleware_;
}

void EventMiddleware::clear() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    middleware_.clear();
    LOG_INFO("Cleared all middleware");
}

void EventMiddleware::sort_middleware() {
    std::sort(middleware_.begin(), middleware_.end(),
        [](const auto& a, const auto& b) {
            return a->get_priority() > b->get_priority();
        });
}

// BuiltInMiddleware implementations

namespace BuiltInMiddleware {

// Authentication middleware

Authentication::Authentication(const std::string& token, bool require_user_id)
    : bot_token_(token), require_user_id_(require_user_id) {
    LOG_INFO("Authentication middleware initialized");
}

bool Authentication::process(const std::string& event_name,
                              const nlohmann::json& event_data,
                              std::function<void()> next) {
    // TODO: Implement actual authentication logic
    // This would validate tokens, check user permissions, etc.
    LOG_DEBUG("Authentication middleware processing event: " + event_name);
    next();
    return true;
}

// PermissionChecker middleware

PermissionChecker::PermissionChecker(const std::unordered_map<std::string, uint64_t>& permissions)
    : required_permissions_(permissions) {
    LOG_INFO("PermissionChecker middleware initialized");
}

bool PermissionChecker::process(const std::string& event_name,
                              const nlohmann::json& event_data,
                              std::function<void()> next) {
    // TODO: Implement actual permission checking logic
    // This would check if the event has required permissions
    LOG_DEBUG("PermissionChecker middleware processing event: " + event_name);
    next();
    return true;
}

// Transformer middleware

Transformer::Transformer(const std::unordered_map<std::string, 
                                     std::function<nlohmann::json(nlohmann::json)>>& transformers)
    : transformers_(transformers) {
    LOG_INFO("Transformer middleware initialized");
}

bool Transformer::process(const std::string& event_name,
                        const nlohmann::json& event_data,
                        std::function<void()> next) {
    auto it = transformers_.find(event_name);
    if (it != transformers_.end()) {
        try {
            auto transformed_data = it->second(event_data);
            // TODO: Pass transformed data to next middleware
            // This requires modifying the event data flow
            LOG_DEBUG("Transformer middleware transformed event: " + event_name);
        } catch (const std::exception& e) {
            LOG_ERROR("Transformer middleware error: " + std::string(e.what()));
        }
    }
    
    next();
    return true;
}

// Filter middleware

Filter::Filter(const std::vector<EventFilter>& filters, const std::string& mode)
    : filters_(filters), filter_mode_(mode) {
    LOG_INFO("Filter middleware initialized with mode: " + mode);
}

bool Filter::process(const std::string& event_name,
                    const nlohmann::json& event_data,
                    std::function<void()> next) {
    bool should_pass = true;
    
    if (filter_mode_ == "all") {
        // All filters must pass
        should_pass = std::all_of(filters_.begin(), filters_.end(),
            [&event_data](const auto& filter) {
                return !filter || filter(event_data);
            });
    } else {
        // Any filter can pass
        should_pass = std::any_of(filters_.begin(), filters_.end(),
            [&event_data](const auto& filter) {
                return !filter || filter(event_data);
            });
    }
    
    if (should_pass) {
        next();
    } else {
        LOG_DEBUG("Filter middleware blocked event: " + event_name);
    }
    
    return should_pass;
}

// Cache middleware

Cache::Cache(size_t max_size, std::chrono::milliseconds ttl)
    : max_cache_size_(max_size), cache_ttl_(ttl) {
    LOG_INFO("Cache middleware initialized");
}

bool Cache::process(const std::string& event_name,
                  const nlohmann::json& event_data,
                  std::function<void()> next) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    cleanup_cache();
    
    // Check if event is already cached
    auto& cache = event_cache_[event_name];
    auto it = std::find_if(cache.begin(), cache.end(),
        [&event_data](const nlohmann::json& cached_event) {
            return cached_event["id"] == event_data["id"];
        });
    
    if (it != cache.end()) {
        LOG_DEBUG("Cache middleware found cached event: " + event_name);
        return false; // Don't process duplicate
    }
    
    // Add to cache
    if (cache.size() >= max_cache_size_) {
        cache.erase(cache.begin()); // Remove oldest
    }
    
    nlohmann::json cached_event = event_data;
    cached_event["cached_at"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    
    cache.push_back(cached_event);
    
    lock.unlock();
    next();
    return true;
}

void Cache::cleanup_cache() {
    auto now = std::chrono::steady_clock::now();
    
    for (auto& [event_name, cache] : event_cache_) {
        auto it = std::remove_if(cache.begin(), cache.end(),
            [now, this](const nlohmann::json& cached_event) {
                if (!cached_event.contains("cached_at")) {
                    return true;
                }
                
                auto cached_at = std::chrono::seconds(cached_event["cached_at"]);
                auto event_time = std::chrono::steady_clock::time_point(
                    std::chrono::duration_cast<std::chrono::steady_clock::duration>(cached_at));
                
                return (now - event_time) > cache_ttl_;
            });
        
        if (it != cache.end()) {
            LOG_DEBUG("Cleaned up expired cached events for: " + event_name);
        }
    }
}

std::vector<nlohmann::json> Cache::get_cached_events(const std::string& event_name) const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    auto it = event_cache_.find(event_name);
    if (it != event_cache_.end()) {
        return it->second;
    }
    
    return {};
}

void Cache::clear_cache(const std::string& event_name) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto it = event_cache_.find(event_name);
    if (it != event_cache_.end()) {
        it->second.clear();
        LOG_DEBUG("Cleared cache for event: " + event_name);
    }
}

// Metrics middleware

Metrics::Metrics() = default;

bool Metrics::process(const std::string& event_name,
                    const nlohmann::json& event_data,
                    std::function<void()> next) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    
    event_counts_[event_name]++;
    last_event_times_[event_name] = now;
    
    LOG_DEBUG("Metrics middleware recorded event: " + event_name);
    next();
    return true;
}

nlohmann::json Metrics::get_metrics() const {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    
    nlohmann::json metrics;
    metrics["event_counts"] = nlohmann::json::object();
    metrics["last_event_times"] = nlohmann::json::object();
    metrics["error_counts"] = error_counts_;
    
    for (const auto& [event_name, count] : event_counts_) {
        metrics["event_counts"][event_name] = count;
    }
    
    for (const auto& [event_name, time] : last_event_times_) {
        metrics["last_event_times"][event_name] = std::chrono::duration_cast<std::chrono::seconds>(
            time.time_since_epoch()).count();
    }
    
    return metrics;
}

void Metrics::reset_metrics() {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    event_counts_.clear();
    last_event_times_.clear();
    error_counts_.clear();
    
    LOG_INFO("Metrics middleware reset");
}

// Debugger middleware

Debugger::Debugger(const std::vector<std::string>& events, bool log_all, bool include_stack)
    : debug_events_(events), log_all_events_(log_all), include_stack_trace_(include_stack) {
    LOG_INFO("Debugger middleware initialized");
}

bool Debugger::process(const std::string& event_name,
                    const nlohmann::json& event_data,
                    std::function<void()> next) {
    bool should_log = log_all_events_ || 
                     std::find(debug_events_.begin(), debug_events_.end(), event_name) != debug_events_.end();
    
    if (should_log) {
        std::string debug_info = "DEBUG EVENT: " + event_name + " | " + event_data.dump();
        
        if (include_stack_trace_) {
            // TODO: Add stack trace information
            debug_info += " | STACK_TRACE";
        }
        
        LOG_DEBUG(debug_info);
    }
    
    next();
    return true;
}

} // namespace BuiltInMiddleware

// MiddlewareFactory implementations

std::shared_ptr<BuiltInMiddleware::Authentication> MiddlewareFactory::create_authentication(
        const std::string& token,
        bool require_user_id) {
    return std::make_shared<BuiltInMiddleware::Authentication>(token, require_user_id);
}

std::shared_ptr<BuiltInMiddleware::PermissionChecker> MiddlewareFactory::create_permission_checker(
        const std::unordered_map<std::string, uint64_t>& permissions) {
    return std::make_shared<BuiltInMiddleware::PermissionChecker>(permissions);
}

std::shared_ptr<BuiltInMiddleware::Transformer> MiddlewareFactory::create_transformer(
        const std::unordered_map<std::string, std::function<nlohmann::json(nlohmann::json)>>& transformers) {
    return std::make_shared<BuiltInMiddleware::Transformer>(transformers);
}

std::shared_ptr<BuiltInMiddleware::Filter> MiddlewareFactory::create_filter(
        const std::vector<EventFilter>& filters,
        const std::string& mode) {
    return std::make_shared<BuiltInMiddleware::Filter>(filters, mode);
}

std::shared_ptr<BuiltInMiddleware::Cache> MiddlewareFactory::create_cache(
        size_t max_size,
        std::chrono::milliseconds ttl) {
    return std::make_shared<BuiltInMiddleware::Cache>(max_size, ttl);
}

std::shared_ptr<BuiltInMiddleware::Metrics> MiddlewareFactory::create_metrics() {
    return std::make_shared<BuiltInMiddleware::Metrics>();
}

std::shared_ptr<BuiltInMiddleware::Debugger> MiddlewareFactory::create_debugger(
        const std::vector<std::string>& events,
        bool log_all,
        bool include_stack) {
    return std::make_shared<BuiltInMiddleware::Debugger>(events, log_all, include_stack);
}

} // namespace discord