#pragma once

#include "../core/interfaces.h"
#include <curl/curl.h>
#include <future>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <atomic>
#include <nlohmann/json.hpp>

namespace discord {

class HTTPClient : public IHttpClient {
private:
    struct Request {
        std::string method;
        std::string url;
        nlohmann::json data;
        IHttpClient::Headers headers;
        std::promise<nlohmann::json> promise;
    };
    
    CURL* curl_;
    std::queue<Request*> request_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::thread worker_thread_;
    std::atomic<bool> running_;
    std::chrono::milliseconds timeout_;
    std::string base_url_;
    std::string token_;
    
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    static size_t HeaderCallback(void* contents, size_t size, size_t nmemb, std::string* userp);
    
    void worker_loop();
    std::string perform_request(const std::string& method, const std::string& url, 
                               const nlohmann::json& data, const IHttpClient::Headers& headers);
    IHttpClient::Headers get_default_headers() const;
    
public:
    explicit HTTPClient(const std::string& token, const std::string& base_url = "https://discord.com/api/v10");
    ~HTTPClient() override;
    
    std::future<nlohmann::json> get(const std::string& url, const IHttpClient::Headers& headers = {}) override;
    std::future<nlohmann::json> post(const std::string& url, const nlohmann::json& data, const IHttpClient::Headers& headers = {}) override;
    std::future<nlohmann::json> put(const std::string& url, const nlohmann::json& data, const IHttpClient::Headers& headers = {}) override;
    std::future<nlohmann::json> patch(const std::string& url, const nlohmann::json& data, const IHttpClient::Headers& headers = {}) override;
    std::future<nlohmann::json> delete_(const std::string& url, const IHttpClient::Headers& headers = {}) override;
    std::future<void> set_timeout(std::chrono::milliseconds timeout) override;
    
    void shutdown();
    void set_base_url(const std::string& url);
    void set_token(const std::string& token);
};

} // namespace discord