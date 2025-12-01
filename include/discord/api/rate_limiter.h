#pragma once

#include <chrono>
#include <string>
#include <unordered_map>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <thread>

namespace discord {

struct RateLimitInfo {
    int remaining = -1;
    int limit = -1;
    std::chrono::steady_clock::time_point reset_time;
    bool global = false;
};

class RateLimiter {
public:
    RateLimiter();
    ~RateLimiter();
    
    void update_limits(const std::string& endpoint, const RateLimitInfo& info);
    bool can_make_request(const std::string& endpoint);
    void wait_if_needed(const std::string& endpoint);
    
    void set_global_limit(std::chrono::milliseconds delay);
    void set_endpoint_limit(const std::string& endpoint, int max_requests, std::chrono::milliseconds window);

private:
    struct EndpointLimit {
        int max_requests;
        std::chrono::milliseconds window;
        std::queue<std::chrono::steady_clock::time_point> request_times;
    };
    
    std::unordered_map<std::string, RateLimitInfo> rate_limits_;
    std::unordered_map<std::string, EndpointLimit> endpoint_limits_;
    std::chrono::steady_clock::time_point global_reset_time_;
    
    mutable std::mutex mutex_;
    std::condition_variable cv_;
    
    void cleanup_old_requests(const std::string& endpoint);
    std::chrono::milliseconds get_wait_time(const std::string& endpoint);
};

class RequestQueue {
public:
    using RequestFunction = std::function<void()>;
    
    RequestQueue();
    ~RequestQueue();
    
    void enqueue(const std::string& endpoint, RequestFunction request);
    void start();
    void stop();
    
    void set_rate_limiter(std::shared_ptr<RateLimiter> rate_limiter);

private:
    struct QueuedRequest {
        std::string endpoint;
        RequestFunction function;
        std::chrono::steady_clock::time_point enqueue_time;
    };
    
    std::queue<QueuedRequest> requests_;
    std::shared_ptr<RateLimiter> rate_limiter_;
    
    std::thread worker_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool running_ = false;
    
    void process_requests();
};

}