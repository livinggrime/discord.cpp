#pragma once

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

namespace discord {

class APIEndpoints {
public:
    static nlohmann::json get_user(const std::string& user_id);
    static nlohmann::json get_current_user();
    static nlohmann::json modify_current_user(const nlohmann::json& data);
    static nlohmann::json get_current_user_guilds();
    static nlohmann::json leave_guild(const std::string& guild_id);
    
    static nlohmann::json get_guild(const std::string& guild_id);
    static nlohmann::json get_guild_channels(const std::string& guild_id);
    static nlohmann::json get_guild_members(const std::string& guild_id, int limit = 1, const std::string& after = "");
    static nlohmann::json get_guild_member(const std::string& guild_id, const std::string& user_id);
    
    static nlohmann::json get_channel(const std::string& channel_id);
    static nlohmann::json modify_channel(const std::string& channel_id, const nlohmann::json& data);
    static nlohmann::json delete_channel(const std::string& channel_id);
    
    static nlohmann::json get_channel_messages(const std::string& channel_id, int limit = 50, const std::string& before = "", const std::string& after = "", const std::string& around = "");
    static nlohmann::json get_channel_message(const std::string& channel_id, const std::string& message_id);
    static nlohmann::json send_message(const std::string& channel_id, const nlohmann::json& data);
    static nlohmann::json edit_message(const std::string& channel_id, const std::string& message_id, const nlohmann::json& data);
    static nlohmann::json delete_message(const std::string& channel_id, const std::string& message_id);
    
    static nlohmann::json create_interaction_response(const std::string& interaction_id, const std::string& interaction_token, const nlohmann::json& data);
    static nlohmann::json get_original_interaction_response(const std::string& interaction_id, const std::string& interaction_token);
    static nlohmann::json edit_original_interaction_response(const std::string& interaction_id, const std::string& interaction_token, const nlohmann::json& data);
    static nlohmann::json delete_original_interaction_response(const std::string& interaction_id, const std::string& interaction_token);
    
    static nlohmann::json get_gateway();
    static nlohmann::json get_gateway_bot();
    
    // Reactions
    static nlohmann::json add_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji);
    static nlohmann::json remove_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji, const std::string& user_id = "");
    
    // Webhooks (for followup messages)
    static nlohmann::json edit_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id, const nlohmann::json& message);
    static nlohmann::json delete_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id);
    
    // Guild roles
    static nlohmann::json create_guild_role(const std::string& guild_id, const nlohmann::json& role_data);
    static nlohmann::json delete_guild_role(const std::string& guild_id, const std::string& role_id);
    static nlohmann::json add_guild_member_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id);
    static nlohmann::json remove_guild_member_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id);
    
    // Channel creation
    static nlohmann::json create_channel(const std::string& guild_id, const nlohmann::json& data);

private:
    APIEndpoints() = default;
};

}