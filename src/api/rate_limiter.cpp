#include <discord/api/rate_limiter.h>
#include <algorithm>

namespace discord {

RateLimiter::RateLimiter() = default;
RateLimiter::~RateLimiter() = default;

void RateLimiter::update_limits(const std::string& endpoint, const RateLimitInfo& info) {
    std::lock_guard<std::mutex> lock(mutex_);
    rate_limits_[endpoint] = info;
    
    if (info.global) {
        global_reset_time_ = info.reset_time;
    }
}

bool RateLimiter::can_make_request(const std::string& endpoint) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto now = std::chrono::steady_clock::now();
    
    if (now < global_reset_time_) {
        return false;
    }
    
    auto it = rate_limits_.find(endpoint);
    if (it != rate_limits_.end() && it->second.remaining >= 0) {
        if (it->second.remaining == 0 && now < it->second.reset_time) {
            return false;
        }
    }
    
    auto endpoint_it = endpoint_limits_.find(endpoint);
    if (endpoint_it != endpoint_limits_.end()) {
        cleanup_old_requests(endpoint);
        if (endpoint_it->second.request_times.size() >= static_cast<size_t>(endpoint_it->second.max_requests)) {
            return false;
        }
    }
    
    return true;
}

void RateLimiter::wait_if_needed(const std::string& endpoint) {
    auto wait_time = get_wait_time(endpoint);
    if (wait_time.count() > 0) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait_for(lock, wait_time);
    }
}

void RateLimiter::set_global_limit(std::chrono::milliseconds delay) {
    std::lock_guard<std::mutex> lock(mutex_);
    global_reset_time_ = std::chrono::steady_clock::now() + delay;
}

void RateLimiter::set_endpoint_limit(const std::string& endpoint, int max_requests, std::chrono::milliseconds window) {
    std::lock_guard<std::mutex> lock(mutex_);
    endpoint_limits_[endpoint] = {max_requests, window, {}};
}

void RateLimiter::cleanup_old_requests(const std::string& endpoint) {
    auto it = endpoint_limits_.find(endpoint);
    if (it == endpoint_limits_.end()) return;
    
    auto now = std::chrono::steady_clock::now();
    auto cutoff = now - it->second.window;
    
    while (!it->second.request_times.empty() && it->second.request_times.front() < cutoff) {
        it->second.request_times.pop();
    }
}

std::chrono::milliseconds RateLimiter::get_wait_time(const std::string& endpoint) {
    auto now = std::chrono::steady_clock::now();
    
    if (now < global_reset_time_) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(global_reset_time_ - now);
    }
    
    auto it = rate_limits_.find(endpoint);
    if (it != rate_limits_.end() && it->second.remaining == 0 && now < it->second.reset_time) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(it->second.reset_time - now);
    }
    
    auto endpoint_it = endpoint_limits_.find(endpoint);
    if (endpoint_it != endpoint_limits_.end()) {
        cleanup_old_requests(endpoint);
        if (endpoint_it->second.request_times.size() >= static_cast<size_t>(endpoint_it->second.max_requests)) {
            auto oldest_request = endpoint_it->second.request_times.front();
            auto reset_time = oldest_request + endpoint_it->second.window;
            if (reset_time > now) {
                return std::chrono::duration_cast<std::chrono::milliseconds>(reset_time - now);
            }
        }
    }
    
    return std::chrono::milliseconds(0);
}

RequestQueue::RequestQueue() = default;
RequestQueue::~RequestQueue() {
    stop();
}

void RequestQueue::enqueue(const std::string& endpoint, RequestFunction request) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        requests_.push({endpoint, std::move(request), std::chrono::steady_clock::now()});
    }
    cv_.notify_one();
}

void RequestQueue::start() {
    if (running_) return;
    
    running_ = true;
    worker_thread_ = std::thread(&RequestQueue::process_requests, this);
}

void RequestQueue::stop() {
    if (!running_) return;
    
    running_ = false;
    cv_.notify_all();
    
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
}

void RequestQueue::set_rate_limiter(std::shared_ptr<RateLimiter> rate_limiter) {
    rate_limiter_ = std::move(rate_limiter);
}

void RequestQueue::process_requests() {
    while (running_) {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !requests_.empty() || !running_; });
        
        if (!running_) break;
        
        if (requests_.empty()) continue;
        
        auto request = requests_.front();
        requests_.pop();
        lock.unlock();
        
        if (rate_limiter_) {
            rate_limiter_->wait_if_needed(request.endpoint);
        }
        
        try {
            request.function();
        } catch (const std::exception& e) {
            // Log error and continue
        }
    }
}

}