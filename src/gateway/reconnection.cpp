#include <discord/gateway/reconnection.h>
#include <random>
#include <algorithm>

namespace discord {

ReconnectionManager::ReconnectionManager() = default;
ReconnectionManager::~ReconnectionManager() {
    stop_reconnecting();
}

void ReconnectionManager::set_callbacks(ReconnectCallback reconnect_cb, ResumeCallback resume_cb) {
    std::lock_guard<std::mutex> lock(mutex_);
    reconnect_callback_ = std::move(reconnect_cb);
    resume_callback_ = std::move(resume_cb);
}

void ReconnectionManager::handle_disconnect(int close_code, const std::string& reason) {
    if (!auto_reconnect_enabled_.load()) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    bool can_resume = is_resumable_close_code(close_code) && session_info_.can_resume;
    
    if (can_resume) {
        session_info_.can_resume = true;
    } else {
        session_info_.can_resume = false;
        session_info_.session_id.clear();
        session_info_.sequence_number = 0;
    }
    
    start_reconnect_sequence();
}

void ReconnectionManager::handle_invalid_session(bool can_resume) {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        session_info_.can_resume = can_resume;
        if (!can_resume) {
            session_info_.session_id.clear();
            session_info_.sequence_number = 0;
        }
    }
    
    start_reconnect_sequence();
}

void ReconnectionManager::handle_connection_restored() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        is_reconnecting_ = false;
        current_retry_count_ = 0;
        should_stop_ = true;
        cv_.notify_all();
    }
    
    if (reconnect_thread_.joinable()) {
        reconnect_thread_.join();
    }
}

void ReconnectionManager::enable_auto_reconnect(bool enabled) {
    auto_reconnect_enabled_ = enabled;
}

void ReconnectionManager::set_max_retries(int max_retries) {
    max_retries_ = max_retries;
}

void ReconnectionManager::set_base_delay(std::chrono::milliseconds delay) {
    base_delay_ = delay;
}

void ReconnectionManager::set_max_delay(std::chrono::milliseconds delay) {
    max_delay_ = delay;
}

bool ReconnectionManager::should_resume() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(mutex_));
    return session_info_.can_resume && !session_info_.session_id.empty();
}

bool ReconnectionManager::is_reconnecting() const {
    return is_reconnecting_;
}

void ReconnectionManager::stop_reconnecting() {
    should_stop_ = true;
    cv_.notify_all();
    
    if (reconnect_thread_.joinable()) {
        reconnect_thread_.join();
    }
}

void ReconnectionManager::reset_session() {
    std::lock_guard<std::mutex> lock(mutex_);
    session_info_.session_id.clear();
    session_info_.sequence_number = 0;
    session_info_.can_resume = false;
}

void ReconnectionManager::attempt_reconnection() {
    bool should_resume = this->should_resume();
    
    if (reconnect_callback_) {
        reconnect_callback_(should_resume);
    }
    
    if (should_resume && resume_callback_) {
        resume_callback_();
    }
}

std::chrono::milliseconds ReconnectionManager::calculate_backoff_delay(int attempt) const {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0.8, 1.2);
    
    auto base = base_delay_.load();
    auto max_delay = max_delay_.load();
    
    auto delay = std::chrono::milliseconds(
        static_cast<long long>(base.count() * std::pow(2, attempt) * dis(gen))
    );
    
    return std::min(delay, max_delay);
}

void ReconnectionManager::exponential_backoff_reconnect() {
    while (!should_stop_.load() && current_retry_count_.load() < max_retries_.load()) {
        auto delay = calculate_backoff_delay(current_retry_count_.load());
        
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (cv_.wait_for(lock, delay, [this] { return should_stop_.load(); })) {
                return;
            }
        }
        
        if (should_stop_.load()) break;
        
        attempt_reconnection();
        current_retry_count_++;
        
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    
    is_reconnecting_ = false;
}

bool ReconnectionManager::is_resumable_close_code(int close_code) const {
    switch (close_code) {
        case 1000: // Normal Closure
        case 1001: // Going Away
        case 1006: // Abnormal Closure
        case 1009: // Message Too Big
        case 1011: // Internal Server Error
        case 1012: // Service Restart
        case 1013: // Try Again Later
        case 1014: // Bad Gateway
            return true;
        case 4000: // Unknown Error
        case 4001: // Unknown Opcode
        case 4002: // Decode Error
        case 4003: // Not Authenticated
        case 4004: // Authentication Failed
        case 4005: // Already Authenticated
        case 4007: // Invalid Seq
        case 4008: // Rate Limited
        case 4009: // Session Timed Out
        case 4010: // Invalid Shard
        case 4011: // Sharding Required
        case 4012: // Invalid API Version
        case 4013: // Invalid Intents
        case 4014: // Disallowed Intents
            return false;
        default:
            return close_code >= 1000 && close_code < 2000;
    }
}

void ReconnectionManager::start_reconnect_sequence() {
    if (is_reconnecting_.load()) {
        return;
    }
    
    is_reconnecting_ = true;
    current_retry_count_ = 0;
    should_stop_ = false;
    
    if (reconnect_thread_.joinable()) {
        reconnect_thread_.join();
    }
    
    reconnect_thread_ = std::thread(&ReconnectionManager::exponential_backoff_reconnect, this);
}

}