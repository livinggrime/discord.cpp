#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace discord {

class Auth {
public:
    static std::string bot_token(const std::string& token);
    static std::string bearer_token(const std::string& token);
    
    static bool validate_token(const std::string& token);
    static nlohmann::json get_current_user(const std::string& token);
    
private:
    Auth() = default;
};

}