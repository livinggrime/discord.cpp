#pragma once

#include <string>
#include <chrono>
#include <functional>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace discord {

class ReconnectionManager {
public:
    using ReconnectCallback = std::function<void(bool)>;
    using ResumeCallback = std::function<void()>;
    
    ReconnectionManager();
    ~ReconnectionManager();
    
    void set_callbacks(ReconnectCallback reconnect_cb, ResumeCallback resume_cb);
    
    void handle_disconnect(int close_code, const std::string& reason);
    void handle_invalid_session(bool can_resume);
    void handle_connection_restored();
    
    void enable_auto_reconnect(bool enabled);
    void set_max_retries(int max_retries);
    void set_base_delay(std::chrono::milliseconds delay);
    void set_max_delay(std::chrono::milliseconds delay);
    
    bool should_resume() const;
    bool is_reconnecting() const;
    void stop_reconnecting();
    
    void reset_session();

private:
    struct SessionInfo {
        std::string session_id;
        int sequence_number = 0;
        bool can_resume = false;
        std::chrono::steady_clock::time_point last_heartbeat;
    };
    
    ReconnectCallback reconnect_callback_;
    ResumeCallback resume_callback_;
    
    SessionInfo session_info_;
    
    std::atomic<bool> auto_reconnect_enabled_{true};
    std::atomic<int> max_retries_{5};
    std::atomic<std::chrono::milliseconds> base_delay_{std::chrono::milliseconds(1000)};
    std::atomic<std::chrono::milliseconds> max_delay_{std::chrono::milliseconds(30000)};
    
    std::atomic<bool> is_reconnecting_{false};
    std::atomic<int> current_retry_count_{0};
    
    std::thread reconnect_thread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> should_stop_{false};
    
    void attempt_reconnection();
    std::chrono::milliseconds calculate_backoff_delay(int attempt) const;
    void exponential_backoff_reconnect();
    
    bool is_resumable_close_code(int close_code) const;
    void start_reconnect_sequence();
};

}