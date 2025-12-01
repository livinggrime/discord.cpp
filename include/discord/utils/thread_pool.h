#pragma once

#include "../core/interfaces.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <functional>

namespace discord {

class ThreadPool : public IThreadPool {
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable condition_;
    std::atomic<bool> stop_;
    size_t thread_count_;
    
public:
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());
    ~ThreadPool() override;
    
    std::future<void> submit(std::function<void()> task) override;
    void shutdown() override;
    size_t get_thread_count() const override;
    size_t get_pending_tasks() const override;
    
private:
    void worker_thread();
};

} // namespace discord