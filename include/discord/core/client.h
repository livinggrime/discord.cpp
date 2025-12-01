#pragma once

#include "interfaces.h"
#include "../utils/types.h"
#include <memory>
#include <functional>
#include <future>
#include <string>
#include <nlohmann/json.hpp>

namespace discord {

// Forward declarations
class ICache;
class IHttpClient;
class IWebSocketClient;
class IRateLimiter;
class ILogger;
class IThreadPool;

struct User;
struct Guild;
struct Channel;
struct Message;

// Main Discord client interface
class DiscordClient {
public:
    explicit DiscordClient(std::string token);
    ~DiscordClient();
    
    // Connection management
    bool connect();
    void disconnect();
    bool is_connected() const;
    bool is_ready() const;
    
    // Event handlers
    void on_ready(std::function<void()> callback);
    void on_message(std::function<void(const nlohmann::json&)> callback);
    void on_message_deleted(std::function<void(const std::string&, const std::string&)> callback);
    void on_message_updated(std::function<void(const nlohmann::json&, const nlohmann::json&)> callback);
    void on_interaction(std::function<void(const nlohmann::json&)> callback);
    void on_guild_join(std::function<void(const nlohmann::json&)> callback);
    void on_guild_leave(std::function<void(const std::string&)> callback);
    void on_member_join(std::function<void(const nlohmann::json&)> callback);
    void on_member_leave(std::function<void(const nlohmann::json&)> callback);
    
    // REST API methods
    nlohmann::json get_user(const std::string& user_id);
    nlohmann::json get_guild(const std::string& guild_id);
    nlohmann::json get_channel(const std::string& channel_id);
    nlohmann::json get_channel_messages(const std::string& channel_id, int limit = 50, const std::string& before = "", const std::string& after = "");
    
    nlohmann::json send_message(const std::string& channel_id, const std::string& content);
    nlohmann::json send_embed(const std::string& channel_id, const nlohmann::json& embed);
    void delete_message(const std::string& channel_id, const std::string& message_id);
    void edit_message(const std::string& channel_id, const std::string& message_id, const std::string& new_content);
    
    void add_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji);
    void remove_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji, const std::string& user_id = "");
    
    void create_interaction_response(const std::string& interaction_id, const std::string& interaction_token, const nlohmann::json& response);
    void edit_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id, const nlohmann::json& message);
    void delete_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id);
    
    // Guild management
    void create_role(const std::string& guild_id, const nlohmann::json& role_data);
    void delete_role(const std::string& guild_id, const std::string& role_id);
    void add_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id);
    void remove_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id);
    
    // Channel management
    nlohmann::json create_text_channel(const std::string& guild_id, const std::string& name, const std::string& parent_id = "", int position = 0);
    nlohmann::json create_voice_channel(const std::string& guild_id, const std::string& name, int bitrate = 64000, int user_limit = 0);
    void delete_channel(const std::string& channel_id);
    void edit_channel(const std::string& channel_id, const nlohmann::json& data);
    
    // Configuration
    void set_token(const std::string& token);
    const std::string& get_token() const;
    void set_intents(int intents);
    int get_intents() const;

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

// Factory for creating clients
class DiscordClientFactory {
public:
    static std::unique_ptr<DiscordClient> create(const std::string& token);
    static std::unique_ptr<DiscordClient> create_with_config(const nlohmann::json& config);
};

} // namespace discord