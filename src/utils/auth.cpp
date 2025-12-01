#include <discord/utils/auth.h>
#include <discord/api/http_client.h>

namespace discord {

std::string Auth::bot_token(const std::string& token) {
    return "Bot " + token;
}

std::string Auth::bearer_token(const std::string& token) {
    return "Bearer " + token;
}

bool Auth::validate_token(const std::string& token) {
    HTTPClient client(token);
    auto response = client.get("/users/@me");
    return !response.is_null();
}

nlohmann::json Auth::get_current_user(const std::string& token) {
    HTTPClient client(token);
    return client.get("/users/@me");
}

}