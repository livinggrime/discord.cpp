#include <discord/api/http_client.h>
#include <sstream>
#include <iostream>
#include <future>
#include <nlohmann/json.hpp>

namespace discord {

HTTPClient::HTTPClient(const std::string& token, const std::string& base_url) 
    : curl_(nullptr), running_(true), timeout_(30000), base_url_(base_url), token_(token) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl_ = curl_easy_init();
    
    if (!curl_) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    worker_thread_ = std::thread(&HTTPClient::worker_loop, this);
}

HTTPClient::~HTTPClient() {
    shutdown();
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
    curl_global_cleanup();
}

size_t HTTPClient::WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

size_t HTTPClient::HeaderCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append((char*)contents, size * nmemb);
    return size * nmemb;
}

void HTTPClient::worker_loop() {
    while (running_) {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        queue_cv_.wait(lock, [this] { return !request_queue_.empty() || !running_; });
        
        if (!running_) break;
        
        auto* request = request_queue_.front();
        request_queue_.pop();
        lock.unlock();
        
        try {
            std::string response = perform_request(request->method, request->url, 
                                                  request->data, request->headers);
            
            nlohmann::json json_response;
            if (!response.empty()) {
                json_response = nlohmann::json::parse(response);
            }
            
            request->promise.set_value(json_response);
        } catch (const std::exception& e) {
            request->promise.set_exception(std::current_exception());
        }
        
        delete request;
    }
}

std::string HTTPClient::perform_request(const std::string& method, const std::string& url, 
                                         const nlohmann::json& data, const IHttpClient::Headers& headers) {
    std::string response;
    std::string header_response;
    
    curl_easy_reset(curl_);
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_, CURLOPT_HEADERFUNCTION, HeaderCallback);
    curl_easy_setopt(curl_, CURLOPT_HEADERDATA, &header_response);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT_MS, timeout_.count());
    curl_easy_setopt(curl_, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl_, CURLOPT_SSL_VERIFYHOST, 2L);
    
    // Set default headers including authorization
    IHttpClient::Headers all_headers = get_default_headers();
    for (const auto& header : headers) {
        all_headers.push_back(header);
    }
    
    // Set headers
    struct curl_slist* chunk = nullptr;
    for (const auto& header : all_headers) {
        std::string header_str = header.first + ": " + header.second;
        chunk = curl_slist_append(chunk, header_str.c_str());
    }
    
    if (method == "GET") {
        curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);
    } else if (method == "POST") {
        curl_easy_setopt(curl_, CURLOPT_POST, 1L);
        std::string json_str = data.dump();
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, json_str.c_str());
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
    } else if (method == "PUT") {
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PUT");
        std::string json_str = data.dump();
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, json_str.c_str());
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
    } else if (method == "PATCH") {
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "PATCH");
        std::string json_str = data.dump();
        curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, json_str.c_str());
        chunk = curl_slist_append(chunk, "Content-Type: application/json");
    } else if (method == "DELETE") {
        curl_easy_setopt(curl_, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    
    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, chunk);
    
    CURLcode res = curl_easy_perform(curl_);
    
    curl_slist_free_all(chunk);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("CURL error: " + std::string(curl_easy_strerror(res)));
    }
    
    long response_code;
    curl_easy_getinfo(curl_, CURLINFO_RESPONSE_CODE, &response_code);
    
    if (response_code >= 400) {
        std::string error_msg = "HTTP error " + std::to_string(response_code);
        if (!response.empty()) {
            try {
                auto json_response = nlohmann::json::parse(response);
                if (json_response.contains("message")) {
                    error_msg += ": " + json_response["message"].get<std::string>();
                }
            } catch (...) {
                // Ignore JSON parsing errors for error messages
            }
        }
        throw std::runtime_error(error_msg);
    }
    
    return response;
}

std::future<nlohmann::json> HTTPClient::get(const std::string& url, const IHttpClient::Headers& headers) {
    auto request = new Request{"GET", base_url_ + url, {}, headers, {}};
    auto future = request->promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        request_queue_.push(request);
    }
    queue_cv_.notify_one();
    
    return future;
}

std::future<nlohmann::json> HTTPClient::post(const std::string& url, const nlohmann::json& data, const IHttpClient::Headers& headers) {
    auto request = new Request{"POST", base_url_ + url, data, headers, {}};
    auto future = request->promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        request_queue_.push(request);
    }
    queue_cv_.notify_one();
    
    return future;
}

std::future<nlohmann::json> HTTPClient::put(const std::string& url, const nlohmann::json& data, const IHttpClient::Headers& headers) {
    auto request = new Request{"PUT", base_url_ + url, data, headers, {}};
    auto future = request->promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        request_queue_.push(request);
    }
    queue_cv_.notify_one();
    
    return future;
}

std::future<nlohmann::json> HTTPClient::patch(const std::string& url, const nlohmann::json& data, const IHttpClient::Headers& headers) {
    auto request = new Request{"PATCH", base_url_ + url, data, headers, {}};
    auto future = request->promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        request_queue_.push(request);
    }
    queue_cv_.notify_one();
    
    return future;
}

std::future<nlohmann::json> HTTPClient::delete_(const std::string& url, const IHttpClient::Headers& headers) {
    auto request = new Request{"DELETE", base_url_ + url, {}, headers, {}};
    auto future = request->promise.get_future();
    
    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        request_queue_.push(request);
    }
    queue_cv_.notify_one();
    
    return future;
}

std::future<void> HTTPClient::set_timeout(std::chrono::milliseconds timeout) {
    auto promise = std::promise<void>();
    auto future = promise.get_future();
    
    timeout_ = timeout;
    promise.set_value();
    
    return future;
}

void HTTPClient::shutdown() {
    running_ = false;
    queue_cv_.notify_all();
    
    if (worker_thread_.joinable()) {
        worker_thread_.join();
    }
    
    // Clear remaining requests
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!request_queue_.empty()) {
        auto* request = request_queue_.front();
        request_queue_.pop();
        request->promise.set_exception(std::make_exception_ptr(
            std::runtime_error("HTTP client shutting down")));
        delete request;
    }
}

IHttpClient::Headers HTTPClient::get_default_headers() const {
    IHttpClient::Headers headers;
    headers.emplace_back("Authorization", "Bot " + token_);
    headers.emplace_back("User-Agent", "DiscordBot (https://github.com/discordcpp/discord.cpp, 1.0.0)");
    headers.emplace_back("Content-Type", "application/json");
    return headers;
}

void HTTPClient::set_base_url(const std::string& url) {
    base_url_ = url;
}

void HTTPClient::set_token(const std::string& token) {
    token_ = token;
}

} // namespace discord