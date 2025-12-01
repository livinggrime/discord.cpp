#include <discord/api/rest_endpoints.h>
#include <discord/api/http_client.h>
#include <memory>
#include <cstdlib>

namespace discord {

static HTTPClient* get_http_client() {
    static HTTPClient* client = nullptr;
    if (!client) {
        const char* token = std::getenv("DISCORD_BOT_TOKEN");
        if (!token) {
            throw std::runtime_error("DISCORD_BOT_TOKEN environment variable not set");
        }
        client = new HTTPClient(token);
    }
    return client;
}

nlohmann::json APIEndpoints::get_user(const std::string& user_id) {
    return get_http_client()->get("/users/" + user_id).get();
}

nlohmann::json APIEndpoints::get_current_user() {
    return get_http_client()->get("/users/@me").get();
}

nlohmann::json APIEndpoints::modify_current_user(const nlohmann::json& data) {
    return get_http_client()->patch("/users/@me", data).get();
}

nlohmann::json APIEndpoints::get_current_user_guilds() {
    return get_http_client()->get("/users/@me/guilds").get();
}

nlohmann::json APIEndpoints::leave_guild(const std::string& guild_id) {
    return get_http_client()->delete_("/users/@me/guilds/" + guild_id).get();
}

nlohmann::json APIEndpoints::get_guild(const std::string& guild_id) {
    return get_http_client()->get("/guilds/" + guild_id).get();
}

nlohmann::json APIEndpoints::get_guild_channels(const std::string& guild_id) {
    return get_http_client()->get("/guilds/" + guild_id + "/channels").get();
}

nlohmann::json APIEndpoints::get_guild_members(const std::string& guild_id, int limit, const std::string& after) {
    std::string endpoint = "/guilds/" + guild_id + "/members?limit=" + std::to_string(limit);
    if (!after.empty()) {
        endpoint += "&after=" + after;
    }
    return get_http_client()->get(endpoint).get();
}

nlohmann::json APIEndpoints::get_guild_member(const std::string& guild_id, const std::string& user_id) {
    return get_http_client()->get("/guilds/" + guild_id + "/members/" + user_id).get();
}

nlohmann::json APIEndpoints::get_channel(const std::string& channel_id) {
    return get_http_client()->get("/channels/" + channel_id).get();
}

nlohmann::json APIEndpoints::modify_channel(const std::string& channel_id, const nlohmann::json& data) {
    return get_http_client()->patch("/channels/" + channel_id, data).get();
}

nlohmann::json APIEndpoints::delete_channel(const std::string& channel_id) {
    return get_http_client()->delete_("/channels/" + channel_id).get();
}

nlohmann::json APIEndpoints::get_channel_messages(const std::string& channel_id, int limit, const std::string& before, const std::string& after, const std::string& around) {
    std::string endpoint = "/channels/" + channel_id + "/messages?limit=" + std::to_string(limit);
    if (!before.empty()) endpoint += "&before=" + before;
    if (!after.empty()) endpoint += "&after=" + after;
    if (!around.empty()) endpoint += "&around=" + around;
    return get_http_client()->get(endpoint).get();
}

nlohmann::json APIEndpoints::get_channel_message(const std::string& channel_id, const std::string& message_id) {
    return get_http_client()->get("/channels/" + channel_id + "/messages/" + message_id).get();
}

nlohmann::json APIEndpoints::send_message(const std::string& channel_id, const nlohmann::json& data) {
    return get_http_client()->post("/channels/" + channel_id + "/messages", data).get();
}

nlohmann::json APIEndpoints::edit_message(const std::string& channel_id, const std::string& message_id, const nlohmann::json& data) {
    return get_http_client()->patch("/channels/" + channel_id + "/messages/" + message_id, data).get();
}

nlohmann::json APIEndpoints::delete_message(const std::string& channel_id, const std::string& message_id) {
    return get_http_client()->delete_("/channels/" + channel_id + "/messages/" + message_id).get();
}

nlohmann::json APIEndpoints::create_interaction_response(const std::string& interaction_id, const std::string& interaction_token, const nlohmann::json& data) {
    return get_http_client()->post("/interactions/" + interaction_id + "/" + interaction_token + "/callback", data).get();
}

nlohmann::json APIEndpoints::get_original_interaction_response(const std::string& interaction_id, const std::string& interaction_token) {
    return get_http_client()->get("/webhooks/" + interaction_id + "/" + interaction_token + "/messages/@original").get();
}

nlohmann::json APIEndpoints::edit_original_interaction_response(const std::string& interaction_id, const std::string& interaction_token, const nlohmann::json& data) {
    return get_http_client()->patch("/webhooks/" + interaction_id + "/" + interaction_token + "/messages/@original", data).get();
}

nlohmann::json APIEndpoints::delete_original_interaction_response(const std::string& interaction_id, const std::string& interaction_token) {
    return get_http_client()->delete_("/webhooks/" + interaction_id + "/" + interaction_token + "/messages/@original").get();
}

nlohmann::json APIEndpoints::get_gateway() {
    return get_http_client()->get("/gateway").get();
}

nlohmann::json APIEndpoints::get_gateway_bot() {
    return get_http_client()->get("/gateway/bot").get();
}

// Reactions
nlohmann::json APIEndpoints::add_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji) {
    return get_http_client()->put("/channels/" + channel_id + "/messages/" + message_id + "/reactions/" + emoji + "/@me", {}).get();
}

nlohmann::json APIEndpoints::remove_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji, const std::string& user_id) {
    std::string endpoint = "/channels/" + channel_id + "/messages/" + message_id + "/reactions/" + emoji;
    if (!user_id.empty()) {
        endpoint += "/" + user_id;
    } else {
        endpoint += "/@me";
    }
    return get_http_client()->delete_(endpoint).get();
}

// Webhooks
nlohmann::json APIEndpoints::edit_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id, const nlohmann::json& message) {
    return get_http_client()->patch("/webhooks/" + application_id + "/" + interaction_token + "/messages/" + message_id, message).get();
}

nlohmann::json APIEndpoints::delete_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id) {
    return get_http_client()->delete_("/webhooks/" + application_id + "/" + interaction_token + "/messages/" + message_id).get();
}

// Guild roles
nlohmann::json APIEndpoints::create_guild_role(const std::string& guild_id, const nlohmann::json& role_data) {
    return get_http_client()->post("/guilds/" + guild_id + "/roles", role_data).get();
}

nlohmann::json APIEndpoints::delete_guild_role(const std::string& guild_id, const std::string& role_id) {
    return get_http_client()->delete_("/guilds/" + guild_id + "/roles/" + role_id).get();
}

nlohmann::json APIEndpoints::add_guild_member_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
    return get_http_client()->put("/guilds/" + guild_id + "/members/" + user_id + "/roles/" + role_id, {}).get();
}

nlohmann::json APIEndpoints::remove_guild_member_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
    return get_http_client()->delete_("/guilds/" + guild_id + "/members/" + user_id + "/roles/" + role_id).get();
}

// Channel creation
nlohmann::json APIEndpoints::create_channel(const std::string& guild_id, const nlohmann::json& data) {
    return get_http_client()->post("/guilds/" + guild_id + "/channels", data).get();
}

} // namespace discord