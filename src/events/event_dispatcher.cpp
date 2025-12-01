#include <discord/events/event_dispatcher.h>
#include <discord/utils/logger.h>
#include <algorithm>
#include <random>
#include <regex>
#include <condition_variable>

namespace discord {

// EventCollector template implementations

template<typename T>
std::string EventCollector<T>::start(EventDispatcher* dispatcher, const std::string& event_name) {
    if (is_active_) {
        return handler_id_;
    }
    
    std::string collector_id = "collector_" + std::to_string(reinterpret_cast<uintptr_t>(this));
    handler_id_ = collector_id;
    is_active_ = true;
    start_time_ = std::chrono::steady_clock::now();
    
    // Register collector callback with dispatcher
    auto callback = [this](const nlohmann::json& event) {
        if (!is_active_) {
            return;
        }
        
        try {
            T typed_event = event; // This requires T to be constructible from json
            if (!filter_ || filter_(typed_event)) {
                std::unique_lock<std::shared_mutex> lock(mutex_);
                collected_items_.push_back(std::move(typed_event));
                
                if (should_stop_collecting()) {
                    lock.unlock();
                    stop();
                }
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Event collector error: " + std::string(e.what()));
        }
    };
    
    dispatcher->on(event_name, std::move(callback), 0, collector_id);
    return collector_id;
}

template<typename T>
void EventCollector<T>::stop() {
    if (!is_active_) {
        return;
    }
    
    is_active_ = false;
    
    // TODO: Remove handler from dispatcher
    // This requires dispatcher to provide removal by ID
}

template<typename T>
bool EventCollector<T>::process_event(const nlohmann::json& event) {
    if (!is_active_) {
        return false;
    }
    
    if (is_timed_out()) {
        if (config_.dispose_on_timeout) {
            stop();
        }
        return false;
    }
    
    try {
        T typed_event = event;
        if (!filter_ || filter_(typed_event)) {
            std::unique_lock<std::shared_mutex> lock(mutex_);
            collected_items_.push_back(std::move(typed_event));
            
            return should_stop_collecting();
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Event collector processing error: " + std::string(e.what()));
    }
    
    return false;
}

template<typename T>
std::optional<T> EventCollector<T>::wait_for_first(std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    
    while (is_active_ && std::chrono::steady_clock::now() < deadline) {
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            if (!collected_items_.empty()) {
                return collected_items_.front();
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return first();
}

template<typename T>
std::vector<T> EventCollector<T>::wait_for_all(std::chrono::milliseconds timeout) {
    auto deadline = std::chrono::steady_clock::now() + timeout;
    
    while (is_active_ && std::chrono::steady_clock::now() < deadline) {
        {
            std::shared_lock<std::shared_mutex> lock(mutex_);
            if (should_stop_collecting()) {
                return collected_items_;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return get_collected();
}

// EventDispatcher implementation

EventDispatcher::EventDispatcher() 
    : start_time_(std::chrono::steady_clock::now()) {
    LOG_INFO("EventDispatcher initialized");
}

EventDispatcher::~EventDispatcher() {
    clear();
    LOG_INFO("EventDispatcher destroyed");
}

std::string EventDispatcher::on(const std::string& event_name, 
                               EventCallback callback,
                               int priority,
                               const std::string& handler_id,
                               bool once) {
    std::string final_handler_id = handler_id;
    if (final_handler_id.empty()) {
        final_handler_id = "handler_" + std::to_string(reinterpret_cast<uintptr_t>(callback.target_type()));
    }
    
    std::unique_lock<std::shared_mutex> lock(handlers_mutex_);
    
    EventHandlerInfo info(std::move(callback), priority, final_handler_id, once);
    handlers_[event_name].push_back(std::move(info));
    
    // Sort handlers by priority (higher priority first)
    sort_handlers(handlers_[event_name]);
    
    LOG_DEBUG("Registered handler for event: " + event_name + " with ID: " + final_handler_id);
    return final_handler_id;
}

bool EventDispatcher::off(const std::string& event_name, const std::string& handler_id) {
    std::unique_lock<std::shared_mutex> lock(handlers_mutex_);
    
    auto it = handlers_.find(event_name);
    if (it == handlers_.end()) {
        return false;
    }
    
    auto& handlers = it->second;
    auto handler_it = std::remove_if(handlers.begin(), handlers.end(),
        [&handler_id](const EventHandlerInfo& info) {
            return info.id == handler_id;
        });
    
    bool removed = handler_it != handlers.end();
    if (removed) {
        LOG_DEBUG("Removed handler for event: " + event_name + " with ID: " + handler_id);
    }
    
    return removed;
}

size_t EventDispatcher::off_all(const std::string& event_name) {
    std::unique_lock<std::shared_mutex> lock(handlers_mutex_);
    
    auto it = handlers_.find(event_name);
    if (it == handlers_.end()) {
        return 0;
    }
    
    size_t count = it->second.size();
    handlers_.erase(it);
    
    LOG_DEBUG("Removed all " + std::to_string(count) + " handlers for event: " + event_name);
    return count;
}

void EventDispatcher::emit(const std::string& event_name, const nlohmann::json& event_data) {
    update_stats();
    
    execute_middleware_chain(event_name, event_data, [this, &event_name, &event_data]() {
        std::shared_lock<std::shared_mutex> lock(handlers_mutex_);
        
        auto it = handlers_.find(event_name);
        if (it == handlers_.end()) {
            return;
        }
        
        // Copy handlers to avoid holding lock during execution
        auto handlers = it->second;
        lock.unlock();
        
        std::vector<EventHandlerInfo> to_remove;
        
        for (auto& handler_info : handlers) {
            try {
                handler_info.callback(event_data);
                handlers_executed_++;
                
                if (handler_info.once) {
                    to_remove.push_back(handler_info);
                }
            } catch (const std::exception& e) {
                LOG_ERROR("Event handler error for " + event_name + ": " + std::string(e.what()));
            }
        }
        
        // Remove one-time handlers
        if (!to_remove.empty()) {
            std::unique_lock<std::shared_mutex> write_lock(handlers_mutex_);
            auto& handler_list = handlers_[event_name];
            
            for (const auto& remove_info : to_remove) {
                handler_list.erase(
                    std::remove_if(handler_list.begin(), handler_list.end(),
                        [&remove_info](const EventHandlerInfo& info) {
                            return info.id == remove_info.id;
                        })
                );
            }
        }
    });
}

void EventDispatcher::emit_filtered(const std::string& event_name,
                                const nlohmann::json& event_data,
                                const std::vector<EventFilter>& filters) {
    // Apply all filters - if any returns false, don't emit
    for (const auto& filter : filters) {
        if (!filter || !filter(event_data)) {
            return;
        }
    }
    
    emit(event_name, event_data);
}

std::optional<nlohmann::json> EventDispatcher::wait_for(const std::string& event_name,
                                                    EventFilter filter,
                                                    std::chrono::milliseconds timeout) {
    std::optional<nlohmann::json> result;
    std::condition_variable cv;
    std::mutex cv_mutex;
    bool event_received = false;
    
    auto handler_id = on(event_name, [&result, &event_received, &cv](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            result = event;
            event_received = true;
            cv.notify_one();
        }
    }, 0, "", true); // Remove after first match
    
    std::unique_lock<std::mutex> lock(cv_mutex);
    cv.wait_for(lock, timeout, [&event_received] { return event_received; });
    
    if (!event_received) {
        off(event_name, handler_id);
    }
    
    return result;
}

template<typename T>
std::shared_ptr<EventCollector<T>> EventDispatcher::create_collector(const std::string& event_name,
                                                               typename EventCollector<T>::FilterFunction filter,
                                                               const CollectorConfig& config) {
    auto collector = std::make_shared<EventCollector<T>>(filter, config);
    std::string handler_id = collector->start(this, event_name);
    
    // Track collector for cleanup
    {
        std::unique_lock<std::shared_mutex> lock(collectors_mutex_);
        active_collectors_[handler_id] = collector;
    }
    
    return collector;
}

void EventDispatcher::add_middleware(std::shared_ptr<IEventMiddleware> middleware) {
    std::unique_lock<std::shared_mutex> lock(middleware_mutex_);
    middleware_.push_back(std::move(middleware));
    
    // Sort by priority (higher priority first)
    std::sort(middleware_.begin(), middleware_.end(),
        [](const auto& a, const auto& b) {
            return a->get_priority() > b->get_priority();
        });
    
    LOG_DEBUG("Added middleware: " + middleware->get_name());
}

bool EventDispatcher::remove_middleware(const std::string& middleware_name) {
    std::unique_lock<std::shared_mutex> lock(middleware_mutex_);
    
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

std::vector<EventHandlerInfo> EventDispatcher::get_handlers(const std::string& event_name) const {
    std::shared_lock<std::shared_mutex> lock(handlers_mutex_);
    
    auto it = handlers_.find(event_name);
    if (it == handlers_.end()) {
        return {};
    }
    
    return it->second;
}

nlohmann::json EventDispatcher::get_statistics() const {
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    
    std::shared_lock<std::shared_mutex> handlers_lock(handlers_mutex_);
    std::shared_lock<std::shared_mutex> collectors_lock(collectors_mutex_);
    
    size_t total_handlers = 0;
    for (const auto& [event_name, handlers] : handlers_) {
        total_handlers += handlers.size();
    }
    
    nlohmann::json stats;
    stats["uptime_seconds"] = uptime.count();
    stats["events_dispatched"] = events_dispatched_.load();
    stats["handlers_executed"] = handlers_executed_.load();
    stats["total_handlers"] = total_handlers;
    stats["active_collectors"] = active_collectors_.size();
    stats["event_types"] = nlohmann::json::object();
    
    for (const auto& [event_name, handlers] : handlers_) {
        stats["event_types"][event_name] = handlers.size();
    }
    
    return stats;
}

void EventDispatcher::reset_statistics() {
    events_dispatched_ = 0;
    handlers_executed_ = 0;
    start_time_ = std::chrono::steady_clock::now();
    
    LOG_INFO("EventDispatcher statistics reset");
}

void EventDispatcher::handle_dispatch(const nlohmann::json& payload) {
    if (!payload.contains("t") || !payload.contains("d")) {
        return;
    }
    
    std::string event_name = payload["t"];
    nlohmann::json event_data = payload["d"];
    
    emit(event_name, event_data);
}

size_t EventDispatcher::get_handler_count() const {
    std::shared_lock<std::shared_mutex> lock(handlers_mutex_);
    
    size_t total = 0;
    for (const auto& [event_name, handlers] : handlers_) {
        total += handlers.size();
    }
    
    return total;
}

size_t EventDispatcher::get_active_collector_count() const {
    std::shared_lock<std::shared_mutex> lock(collectors_mutex_);
    return active_collectors_.size();
}

void EventDispatcher::clear() {
    {
        std::unique_lock<std::shared_mutex> handlers_lock(handlers_mutex_);
        handlers_.clear();
    }
    
    {
        std::unique_lock<std::shared_mutex> collectors_lock(collectors_mutex_);
        
        // Stop all active collectors
        for (auto& [handler_id, collector_ptr] : active_collectors_) {
            // TODO: Call stop on collectors when interface is available
            // This requires collectors to be stored with proper type
        }
        
        active_collectors_.clear();
    }
    
    LOG_INFO("EventDispatcher cleared all handlers and collectors");
}

// Private methods

void EventDispatcher::sort_handlers(std::vector<EventHandlerInfo>& handlers) const {
    std::sort(handlers.begin(), handlers.end(),
        [](const EventHandlerInfo& a, const EventHandlerInfo& b) {
            if (a.priority != b.priority) {
                return a.priority > b.priority; // Higher priority first
            }
            return a.created_at < b.created_at; // Earlier handlers first
        });
}

void EventDispatcher::execute_middleware_chain(const std::string& event_name,
                                         const nlohmann::json& event_data,
                                         std::function<void()> final_handler) {
    std::shared_lock<std::shared_mutex> lock(middleware_mutex_);
    
    if (middleware_.empty()) {
        lock.unlock();
        final_handler();
        return;
    }
    
    // Create middleware chain
    auto middleware_chain = middleware_;
    lock.unlock();
    
    size_t index = 0;
    std::function<void()> next_middleware = [&, final_handler]() {
        if (index >= middleware_chain.size()) {
            final_handler();
            return;
        }
        
        auto& middleware = middleware_chain[index];
        index++;
        
        // Continue to next middleware in chain
        auto next = [&, next_middleware]() {
            next_middleware();
        };
        
        if (!middleware->process(event_name, event_data, next)) {
            LOG_DEBUG("Middleware " + middleware->get_name() + " blocked event: " + event_name);
        }
    };
    
    // Start middleware chain
    next_middleware();
}

void EventDispatcher::update_stats() {
    events_dispatched_++;
}

// EventFilters namespace implementations

namespace EventFilters {

EventFilter by_user_id(const std::string& user_id) {
    return [user_id](const nlohmann::json& event) {
        if (event.contains("author") && event["author"].contains("id")) {
            return event["author"]["id"] == user_id;
        }
        return false;
    };
}

EventFilter by_channel_id(const std::string& channel_id) {
    return [channel_id](const nlohmann::json& event) {
        if (event.contains("channel_id")) {
            return event["channel_id"] == channel_id;
        }
        return false;
    };
}

EventFilter by_guild_id(const std::string& guild_id) {
    return [guild_id](const nlohmann::json& event) {
        if (event.contains("guild_id")) {
            return event["guild_id"] == guild_id;
        }
        return false;
    };
}

EventFilter by_content(const std::string& content) {
    return [content](const nlohmann::json& event) {
        if (!event.contains("content")) {
            return false;
        }
        
        // Support wildcards
        if (content.find('*') != std::string::npos) {
            try {
                std::regex pattern(content.replace("*", ".*"));
                return std::regex_match(event["content"], pattern);
            } catch (const std::exception&) {
                return event["content"] == content;
            }
        }
        
        return event["content"] == content;
    };
}

EventFilter by_bot(bool is_bot) {
    return [is_bot](const nlohmann::json& event) {
        if (event.contains("author") && event["author"].contains("bot")) {
            return event["author"]["bot"] == is_bot;
        }
        return false;
    };
}

EventFilter and_filter(const std::vector<EventFilter>& filters) {
    return [filters](const nlohmann::json& event) {
        for (const auto& filter : filters) {
            if (!filter || !filter(event)) {
                return false;
            }
        }
        return true;
    };
}

EventFilter or_filter(const std::vector<EventFilter>& filters) {
    return [filters](const nlohmann::json& event) {
        for (const auto& filter : filters) {
            if (filter && filter(event)) {
                return true;
            }
        }
        return false;
    };
}

EventFilter not_filter(EventFilter filter) {
    return [filter](const nlohmann::json& event) {
        return !filter || !filter(event);
    };
}

} // namespace EventFilters

// EventMiddleware namespace implementations

namespace EventMiddleware {

RateLimiter::RateLimiter(int max_events, std::chrono::milliseconds window)
    : max_events_per_window_(max_events), window_size_(window) {
    LOG_INFO("RateLimiter initialized: " + std::to_string(max_events) + " events per " + 
             std::to_string(window.count()) + "ms");
}

bool RateLimiter::process(const std::string& event_name,
                        const nlohmann::json& event_data,
                        std::function<void()> next) {
    std::unique_lock<std::shared_mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    auto& counter = counters_[event_name];
    
    // Reset counter if window has passed
    if (now - counter.second >= window_size_) {
        counter.first = 0;
        counter.second = now;
    }
    
    if (counter.first >= max_events_per_window_) {
        LOG_DEBUG("Rate limited event: " + event_name);
        return false;
    }
    
    counter.first++;
    lock.unlock();
    
    next();
    return true;
}

Logger::Logger(bool log_all) : log_all_events_(log_all) {
    LOG_INFO("Event Logger middleware initialized");
}

bool Logger::process(const std::string& event_name,
                    const nlohmann::json& event_data,
                    std::function<void()> next) {
    bool should_log = log_all_events_ || 
                     std::find(logged_events_.begin(), logged_events_.end(), event_name) != logged_events_.end();
    
    if (should_log) {
        LOG_DEBUG("Event: " + event_name + " | Data: " + event_data.dump());
    }
    
    next();
    return true;
}

void Validator::add_validator(const std::string& event_name, std::function<bool(const nlohmann::json&)> validator) {
    validators_[event_name] = std::move(validator);
    LOG_DEBUG("Added validator for event: " + event_name);
}

bool Validator::process(const std::string& event_name,
                      const nlohmann::json& event_data,
                      std::function<void()> next) {
    auto it = validators_.find(event_name);
    if (it != validators_.end()) {
        try {
            if (!it->second(event_data)) {
                LOG_WARN("Event validation failed: " + event_name);
                return false;
            }
        } catch (const std::exception& e) {
            LOG_ERROR("Event validation error for " + event_name + ": " + std::string(e.what()));
            return false;
        }
    }
    
    next();
    return true;
}

} // namespace EventMiddleware

} // namespace discord