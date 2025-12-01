#include <discord/utils/thread_pool.h>

namespace discord {

ThreadPool::ThreadPool(size_t threads) : stop_(false), thread_count_(threads) {
    workers_.reserve(threads);
    for (size_t i = 0; i < threads; ++i) {
        workers_.emplace_back(&ThreadPool::worker_thread, this);
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
}

std::future<void> ThreadPool::submit(std::function<void()> task) {
    auto promise = std::promise<void>();
    auto future = promise.get_future();
    
    auto wrapped_task = [promise = std::move(promise), task = std::move(task)]() mutable {
        try {
            task();
            promise.set_value();
        } catch (...) {
            promise.set_exception(std::current_exception());
        }
    };
    
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        if (stop_) {
            throw std::runtime_error("ThreadPool is stopped");
        }
        tasks_.push(std::move(wrapped_task));
    }
    
    condition_.notify_one();
    return future;
}

void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        stop_ = true;
    }
    
    condition_.notify_all();
    
    for (auto& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    
    workers_.clear();
}

size_t ThreadPool::get_thread_count() const {
    return thread_count_;
}

size_t ThreadPool::get_pending_tasks() const {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    return tasks_.size();
}

void ThreadPool::worker_thread() {
    while (true) {
        std::function<void()> task;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            
            if (stop_ && tasks_.empty()) {
                return;
            }
            
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        
        task();
    }
}

} // namespace discord