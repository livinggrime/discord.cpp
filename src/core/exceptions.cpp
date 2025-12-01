#include <discord/core/exceptions.h>
#include <sstream>

namespace discord {

DiscordException::DiscordException(int code, const std::string& message, const nlohmann::json& response)
    : code_(code), message_(message), response_(response) {
    std::ostringstream oss;
    oss << "Discord Error " << code << ": " << message;
    full_message_ = oss.str();
}

const char* DiscordException::what() const noexcept {
    return full_message_.c_str();
}

int DiscordException::get_code() const {
    return code_;
}

const std::string& DiscordException::get_message() const {
    return message_;
}

const nlohmann::json& DiscordException::get_response() const {
    return response_;
}

DiscordException DiscordException::from_response(const nlohmann::json& response) {
    int code = 0;
    std::string message = "Unknown error";
    
    if (response.contains("code")) {
        code = response["code"];
    }
    
    if (response.contains("message")) {
        message = response["message"];
    }
    
    if (code == 0 && response.contains("retry_after")) {
        return RateLimitException(response["retry_after"], message);
    }
    
    return DiscordException(code, message, response);
}

RateLimitException::RateLimitException(int retry_after, const std::string& message)
    : DiscordException(static_cast<int>(ErrorCode::RATE_LIMITED), message), retry_after_(retry_after) {
}

int RateLimitException::get_retry_after() const {
    return retry_after_;
}

WebSocketException::WebSocketException(int close_code, const std::string& message)
    : DiscordException(close_code, message), close_code_(close_code) {
}

int WebSocketException::get_close_code() const {
    return close_code_;
}

AuthenticationException::AuthenticationException(const std::string& message)
    : DiscordException(static_cast<int>(ErrorCode::UNAUTHORIZED), message) {
}

PermissionException::PermissionException(const std::string& message)
    : DiscordException(static_cast<int>(ErrorCode::MISSING_PERMISSIONS), message) {
}

ValidationException::ValidationException(const std::string& message)
    : DiscordException(static_cast<int>(ErrorCode::INVALID_ACCOUNT_TYPE), message) {
}

}