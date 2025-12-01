#include <discord/core/client.h>
#include <discord/api/http_client.h>
#include <discord/gateway/websocket_client.h>
#include <discord/utils/auth.h>
#include <discord/api/rest_endpoints.h>
#include <discord/events/event_dispatcher.h>
#include <discord/utils/types.h>

#include <thread>
#include <chrono>

namespace discord {

class DiscordClient::Impl {
public:
    explicit Impl(std::string token) 
        : token_(std::move(token))
        , http_client_(token_)
        , websocket_client_()
        , event_handler_()
    {
        websocket_client_.set_token(token_);
        websocket_client_.set_intents(
            static_cast<int>(GatewayIntent::GUILDS) |
            static_cast<int>(GatewayIntent::GUILD_MESSAGES) |
            static_cast<int>(GatewayIntent::MESSAGE_CONTENT)
        );
        
        setup_event_handlers();
    }
    
    bool connect() {
        auto gateway_info = APIEndpoints::get_gateway();
        if (gateway_info.contains("url")) {
            std::string gateway_url = gateway_info["url"];
            gateway_url += "?v=10&encoding=json";
            
            if (websocket_client_.connect(gateway_url)) {
                websocket_client_.identify();
                return true;
            }
        }
        return false;
    }
    
    void disconnect() {
        websocket_client_.disconnect();
    }
    
    void on_ready(std::function<void()> callback) {
        ready_callback_ = std::move(callback);
    }
    
    void on_message(std::function<void(const nlohmann::json&)> callback) {
        message_callback_ = std::move(callback);
    }
    
    void on_interaction(std::function<void(const nlohmann::json&)> callback) {
        interaction_callback_ = std::move(callback);
    }
    
    nlohmann::json get_user(const std::string& user_id) {
        return APIEndpoints::get_user(user_id);
    }
    
    nlohmann::json get_guild(const std::string& guild_id) {
        return APIEndpoints::get_guild(guild_id);
    }
    
    nlohmann::json get_channel(const std::string& channel_id) {
        return APIEndpoints::get_channel(channel_id);
    }
    
    bool send_message(const std::string& channel_id, const std::string& content) {
        nlohmann::json data;
        data["content"] = content;
        auto response = APIEndpoints::send_message(channel_id, data);
        return !response.is_null();
    }
    
    bool create_interaction_response(const std::string& interaction_id, const nlohmann::json& response) {
        std::string token = response["token"];
        return !APIEndpoints::create_interaction_response(interaction_id, token, response).is_null();
    }

private:
    void setup_event_handlers() {
        websocket_client_.on_event([this](const nlohmann::json& event) {
            handle_gateway_event(event);
        });
    }
    
    void handle_gateway_event(const nlohmann::json& event) {
        if (!event.contains("op")) return;
        
        int opcode = event["op"];
        
        switch (opcode) {
            case static_cast<int>(GatewayOpcode::DISPATCH):
                handle_dispatch(event);
                break;
            case static_cast<int>(GatewayOpcode::HELLO):
                handle_hello(event);
                break;
            case static_cast<int>(GatewayOpcode::HEARTBEAT_ACK):
                break;
            default:
                break;
        }
    }
    
    void handle_dispatch(const nlohmann::json& event) {
        if (!event.contains("t")) return;
        
        std::string event_type = event["t"];
        nlohmann::json event_data = event["d"];
        
        if (event_type == "READY") {
            if (ready_callback_) {
                ready_callback_();
            }
        } else if (event_type == "MESSAGE_CREATE") {
            if (message_callback_) {
                message_callback_(event_data);
            }
        } else if (event_type == "INTERACTION_CREATE") {
            if (interaction_callback_) {
                interaction_callback_(event_data);
            }
        }
        
        event_handler_.handle_dispatch(event);
    }
    
    void handle_hello(const nlohmann::json& event) {
        if (event.contains("d") && event["d"].contains("heartbeat_interval")) {
            int heartbeat_interval = event["d"]["heartbeat_interval"];
            std::thread([this, heartbeat_interval]() {
                while (websocket_client_.is_connected()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(heartbeat_interval));
                    if (websocket_client_.is_connected()) {
                        nlohmann::json heartbeat;
                        heartbeat["op"] = static_cast<int>(GatewayOpcode::HEARTBEAT);
                        websocket_client_.send(heartbeat);
                    }
                }
            }).detach();
        }
    }
    
    std::string token_;
    HTTPClient http_client_;
    WebSocketClient websocket_client_;
    EventHandler event_handler_;
    
    std::function<void()> ready_callback_;
    std::function<void(const nlohmann::json&)> message_callback_;
    std::function<void(const nlohmann::json&)> interaction_callback_;
    std::function<void(const std::string&, const std::string&)> message_deleted_callback_;
    std::function<void(const nlohmann::json&, const nlohmann::json&)> message_updated_callback_;
    std::function<void(const nlohmann::json&)> guild_join_callback_;
    std::function<void(const std::string&)> guild_leave_callback_;
    std::function<void(const nlohmann::json&)> member_join_callback_;
    std::function<void(const nlohmann::json&)> member_leave_callback_;
    
    // Additional methods
    nlohmann::json get_channel_messages(const std::string& channel_id, int limit, const std::string& before, const std::string& after) {
        return APIEndpoints::get_channel_messages(channel_id, limit, before, after);
    }
    
    nlohmann::json send_embed(const std::string& channel_id, const nlohmann::json& embed) {
        nlohmann::json data;
        data["embed"] = embed;
        return APIEndpoints::send_message(channel_id, data);
    }
    
    void delete_message(const std::string& channel_id, const std::string& message_id) {
        APIEndpoints::delete_message(channel_id, message_id);
    }
    
    void edit_message(const std::string& channel_id, const std::string& message_id, const std::string& new_content) {
        nlohmann::json data;
        data["content"] = new_content;
        APIEndpoints::edit_message(channel_id, message_id, data);
    }
    
    void add_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji) {
        APIEndpoints::add_reaction(channel_id, message_id, emoji);
    }
    
    void remove_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji, const std::string& user_id) {
        APIEndpoints::remove_reaction(channel_id, message_id, emoji, user_id);
    }
    
    void edit_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id, const nlohmann::json& message) {
        APIEndpoints::edit_followup_message(application_id, interaction_token, message_id, message);
    }
    
    void delete_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id) {
        APIEndpoints::delete_followup_message(application_id, interaction_token, message_id);
    }
    
    void create_role(const std::string& guild_id, const nlohmann::json& role_data) {
        APIEndpoints::create_guild_role(guild_id, role_data);
    }
    
    void delete_role(const std::string& guild_id, const std::string& role_id) {
        APIEndpoints::delete_guild_role(guild_id, role_id);
    }
    
    void add_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
        APIEndpoints::add_guild_member_role(guild_id, user_id, role_id);
    }
    
    void remove_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
        APIEndpoints::remove_guild_member_role(guild_id, user_id, role_id);
    }
    
    nlohmann::json create_text_channel(const std::string& guild_id, const std::string& name, const std::string& parent_id, int position) {
        nlohmann::json data;
        data["name"] = name;
        data["type"] = 0; // Text channel
        if (!parent_id.empty()) data["parent_id"] = parent_id;
        if (position > 0) data["position"] = position;
        return APIEndpoints::create_channel(guild_id, data);
    }
    
    nlohmann::json create_voice_channel(const std::string& guild_id, const std::string& name, int bitrate, int user_limit) {
        nlohmann::json data;
        data["name"] = name;
        data["type"] = 2; // Voice channel
        data["bitrate"] = bitrate;
        data["user_limit"] = user_limit;
        return APIEndpoints::create_channel(guild_id, data);
    }
    
    void delete_channel(const std::string& channel_id) {
        APIEndpoints::delete_channel(channel_id);
    }
    
    void edit_channel(const std::string& channel_id, const nlohmann::json& data) {
        APIEndpoints::modify_channel(channel_id, data);
    }
    
    void set_token(const std::string& token) {
        token_ = token;
        websocket_client_.set_token(token);
    }
    
    const std::string& get_token() const {
        return token_;
    }
    
    void set_intents(int intents) {
        websocket_client_.set_intents(intents);
    }
    
    int get_intents() const {
        return static_cast<int>(GatewayIntent::GUILDS) | 
               static_cast<int>(GatewayIntent::GUILD_MESSAGES) | 
               static_cast<int>(GatewayIntent::MESSAGE_CONTENT);
    }
    
    bool is_connected() const {
        return websocket_client_.is_connected();
    }
    
    bool is_ready() const {
        return websocket_client_.is_connected();
    }
    
    void on_message_deleted(std::function<void(const std::string&, const std::string&)> callback) {
        message_deleted_callback_ = std::move(callback);
    }
    
    void on_message_updated(std::function<void(const nlohmann::json&, const nlohmann::json&)> callback) {
        message_updated_callback_ = std::move(callback);
    }
    
    void on_guild_join(std::function<void(const nlohmann::json&)> callback) {
        guild_join_callback_ = std::move(callback);
    }
    
    void on_guild_leave(std::function<void(const std::string&)> callback) {
        guild_leave_callback_ = std::move(callback);
    }
    
    void on_member_join(std::function<void(const nlohmann::json&)> callback) {
        member_join_callback_ = std::move(callback);
    }
    
    void on_member_leave(std::function<void(const nlohmann::json&)> callback) {
        member_leave_callback_ = std::move(callback);
    }
    
    nlohmann::json send_embed(const std::string& channel_id, const nlohmann::json& embed) {
        nlohmann::json data;
        data["embed"] = embed;
        return APIEndpoints::send_message(channel_id, data);
    }
    
    void delete_message(const std::string& channel_id, const std::string& message_id) {
        APIEndpoints::delete_message(channel_id, message_id);
    }
    
    void edit_message(const std::string& channel_id, const std::string& message_id, const std::string& new_content) {
        nlohmann::json data;
        data["content"] = new_content;
        APIEndpoints::edit_message(channel_id, message_id, data);
    }
    
    void add_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji) {
        APIEndpoints::add_reaction(channel_id, message_id, emoji);
    }
    
    void remove_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji, const std::string& user_id) {
        APIEndpoints::remove_reaction(channel_id, message_id, emoji, user_id);
    }
    
    void edit_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id, const nlohmann::json& message) {
        APIEndpoints::edit_followup_message(application_id, interaction_token, message_id, message);
    }
    
    void delete_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id) {
        APIEndpoints::delete_followup_message(application_id, interaction_token, message_id);
    }
    
    void create_role(const std::string& guild_id, const nlohmann::json& role_data) {
        APIEndpoints::create_guild_role(guild_id, role_data);
    }
    
    void delete_role(const std::string& guild_id, const std::string& role_id) {
        APIEndpoints::delete_guild_role(guild_id, role_id);
    }
    
    void add_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
        APIEndpoints::add_guild_member_role(guild_id, user_id, role_id);
    }
    
    void remove_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
        APIEndpoints::remove_guild_member_role(guild_id, user_id, role_id);
    }
    
    nlohmann::json create_text_channel(const std::string& guild_id, const std::string& name, const std::string& parent_id, int position) {
        nlohmann::json data;
        data["name"] = name;
        data["type"] = 0; // Text channel
        if (!parent_id.empty()) data["parent_id"] = parent_id;
        if (position > 0) data["position"] = position;
        return APIEndpoints::create_channel(guild_id, data);
    }
    
    nlohmann::json create_voice_channel(const std::string& guild_id, const std::string& name, int bitrate, int user_limit) {
        nlohmann::json data;
        data["name"] = name;
        data["type"] = 2; // Voice channel
        data["bitrate"] = bitrate;
        data["user_limit"] = user_limit;
        return APIEndpoints::create_channel(guild_id, data);
    }
    
    void delete_channel(const std::string& channel_id) {
        APIEndpoints::delete_channel(channel_id);
    }
    
    void edit_channel(const std::string& channel_id, const nlohmann::json& data) {
        APIEndpoints::modify_channel(channel_id, data);
    }
    
    void set_token(const std::string& token) {
        token_ = token;
        websocket_client_.set_token(token);
    }
    
    const std::string& get_token() const {
        return token_;
    }
    
    void set_intents(int intents) {
        websocket_client_.set_intents(intents);
    }
    
    int get_intents() const {
        return static_cast<int>(GatewayIntent::GUILDS) | 
               static_cast<int>(GatewayIntent::GUILD_MESSAGES) | 
               static_cast<int>(GatewayIntent::MESSAGE_CONTENT);
    }
    
    bool is_connected() const {
        return websocket_client_.is_connected();
    }
    
    bool is_ready() const {
        return websocket_client_.is_connected();
    }
    
    void on_message_updated(std::function<void(const nlohmann::json&, const nlohmann::json&)> callback) {
        message_updated_callback_ = std::move(callback);
    }
    
    void on_guild_join(std::function<void(const nlohmann::json&)> callback) {
        guild_join_callback_ = std::move(callback);
    }
    
    void on_guild_leave(std::function<void(const std::string&)> callback) {
        guild_leave_callback_ = std::move(callback);
    }
    
    void on_member_join(std::function<void(const nlohmann::json&)> callback) {
        member_join_callback_ = std::move(callback);
    }
    
    void on_member_leave(std::function<void(const nlohmann::json&)> callback) {
        member_leave_callback_ = std::move(callback);
    }
    
    nlohmann::json get_channel_messages(const std::string& channel_id, int limit, const std::string& before, const std::string& after) {
        return APIEndpoints::get_channel_messages(channel_id, limit, before, after);
    }
    
    nlohmann::json send_embed(const std::string& channel_id, const nlohmann::json& embed) {
        nlohmann::json data;
        data["embed"] = embed;
        return APIEndpoints::send_message(channel_id, data);
    }
    
    void delete_message(const std::string& channel_id, const std::string& message_id) {
        APIEndpoints::delete_message(channel_id, message_id);
    }
    
    void edit_message(const std::string& channel_id, const std::string& message_id, const std::string& new_content) {
        nlohmann::json data;
        data["content"] = new_content;
        APIEndpoints::edit_message(channel_id, message_id, data);
    }
    
    void add_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji) {
        APIEndpoints::add_reaction(channel_id, message_id, emoji);
    }
    
    void remove_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji, const std::string& user_id) {
        APIEndpoints::remove_reaction(channel_id, message_id, emoji, user_id);
    }
    
    void edit_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id, const nlohmann::json& message) {
        APIEndpoints::edit_followup_message(application_id, interaction_token, message_id, message);
    }
    
    void delete_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id) {
        APIEndpoints::delete_followup_message(application_id, interaction_token, message_id);
    }
    
    void create_role(const std::string& guild_id, const nlohmann::json& role_data) {
        APIEndpoints::create_guild_role(guild_id, role_data);
    }
    
    void delete_role(const std::string& guild_id, const std::string& role_id) {
        APIEndpoints::delete_guild_role(guild_id, role_id);
    }
    
    void add_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
        APIEndpoints::add_guild_member_role(guild_id, user_id, role_id);
    }
    
    void remove_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
        APIEndpoints::remove_guild_member_role(guild_id, user_id, role_id);
    }
    
    nlohmann::json create_text_channel(const std::string& guild_id, const std::string& name, const std::string& parent_id, int position) {
        nlohmann::json data;
        data["name"] = name;
        data["type"] = 0; // Text channel
        if (!parent_id.empty()) data["parent_id"] = parent_id;
        if (position > 0) data["position"] = position;
        return APIEndpoints::create_channel(guild_id, data);
    }
    
    nlohmann::json create_voice_channel(const std::string& guild_id, const std::string& name, int bitrate, int user_limit) {
        nlohmann::json data;
        data["name"] = name;
        data["type"] = 2; // Voice channel
        data["bitrate"] = bitrate;
        data["user_limit"] = user_limit;
        return APIEndpoints::create_channel(guild_id, data);
    }
    
    void delete_channel(const std::string& channel_id) {
        APIEndpoints::delete_channel(channel_id);
    }
    
    void edit_channel(const std::string& channel_id, const nlohmann::json& data) {
        APIEndpoints::modify_channel(channel_id, data);
    }
    
    void remove_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji, const std::string& user_id) {
        APIEndpoints::remove_reaction(channel_id, message_id, emoji, user_id);
    }
    
    void edit_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id, const nlohmann::json& message) {
        APIEndpoints::edit_webhook_message(application_id, interaction_token, message_id, message);
    }
    
    void delete_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id) {
        APIEndpoints::delete_webhook_message(application_id, interaction_token, message_id);
    }
    
    void create_role(const std::string& guild_id, const nlohmann::json& role_data) {
        APIEndpoints::create_guild_role(guild_id, role_data);
    }
    
    void delete_role(const std::string& guild_id, const std::string& role_id) {
        APIEndpoints::delete_guild_role(guild_id, role_id);
    }
    
    void add_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
        APIEndpoints::add_guild_member_role(guild_id, user_id, role_id);
    }
    
    void remove_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
        APIEndpoints::remove_guild_member_role(guild_id, user_id, role_id);
    }
    
    nlohmann::json create_text_channel(const std::string& guild_id, const std::string& name, const std::string& parent_id, int position) {
        nlohmann::json data;
        data["name"] = name;
        data["type"] = 0; // Text channel
        if (!parent_id.empty()) data["parent_id"] = parent_id;
        if (position > 0) data["position"] = position;
        return APIEndpoints::create_channel(guild_id, data);
    }
    
    nlohmann::json create_voice_channel(const std::string& guild_id, const std::string& name, int bitrate, int user_limit) {
        nlohmann::json data;
        data["name"] = name;
        data["type"] = 2; // Voice channel
        data["bitrate"] = bitrate;
        data["user_limit"] = user_limit;
        return APIEndpoints::create_channel(guild_id, data);
    }
    
    void delete_channel(const std::string& channel_id) {
        APIEndpoints::delete_channel(channel_id);
    }
    
    void edit_channel(const std::string& channel_id, const nlohmann::json& data) {
        APIEndpoints::modify_channel(channel_id, data);
    }
    
    void set_token(const std::string& token) {
        token_ = token;
        websocket_client_.set_token(token);
    }
    
    const std::string& get_token() const {
        return token_;
    }
    
    void set_intents(int intents) {
        websocket_client_.set_intents(intents);
    }
    
    int get_intents() const {
        return static_cast<int>(GatewayIntent::GUILDS) | 
               static_cast<int>(GatewayIntent::GUILD_MESSAGES) | 
               static_cast<int>(GatewayIntent::MESSAGE_CONTENT);
    }
    
    bool is_connected() const {
        return websocket_client_.is_connected();
    }
    
    bool is_ready() const {
        return websocket_client_.is_connected();
    }
};

DiscordClient::DiscordClient(std::string token) 
    : pImpl(std::make_unique<Impl>(std::move(token))) {}

DiscordClient::~DiscordClient() = default;

bool DiscordClient::connect() {
    return pImpl->connect();
}

void DiscordClient::disconnect() {
    pImpl->disconnect();
}

void DiscordClient::on_ready(std::function<void()> callback) {
    pImpl->on_ready(std::move(callback));
}

void DiscordClient::on_message(std::function<void(const nlohmann::json&)> callback) {
    pImpl->on_message(std::move(callback));
}

void DiscordClient::on_interaction(std::function<void(const nlohmann::json&)> callback) {
    pImpl->on_interaction(std::move(callback));
}

nlohmann::json DiscordClient::get_user(const std::string& user_id) {
    return pImpl->get_user(user_id);
}

nlohmann::json DiscordClient::get_guild(const std::string& guild_id) {
    return pImpl->get_guild(guild_id);
}

nlohmann::json DiscordClient::get_channel(const std::string& channel_id) {
    return pImpl->get_channel(channel_id);
}

bool DiscordClient::send_message(const std::string& channel_id, const std::string& content) {
    return pImpl->send_message(channel_id, content);
}

bool DiscordClient::create_interaction_response(const std::string& interaction_id, const nlohmann::json& response) {
    return pImpl->create_interaction_response(interaction_id, response);
}

void DiscordClient::on_message_deleted(std::function<void(const std::string&, const std::string&)> callback) {
    pImpl->on_message_deleted(std::move(callback));
}

void DiscordClient::on_message_updated(std::function<void(const nlohmann::json&, const nlohmann::json&)> callback) {
    pImpl->on_message_updated(std::move(callback));
}

void DiscordClient::on_guild_join(std::function<void(const nlohmann::json&)> callback) {
    pImpl->on_guild_join(std::move(callback));
}

void DiscordClient::on_guild_leave(std::function<void(const std::string&)> callback) {
    pImpl->on_guild_leave(std::move(callback));
}

void DiscordClient::on_member_join(std::function<void(const nlohmann::json&)> callback) {
    pImpl->on_member_join(std::move(callback));
}

void DiscordClient::on_member_leave(std::function<void(const nlohmann::json&)> callback) {
    pImpl->on_member_leave(std::move(callback));
}

nlohmann::json DiscordClient::get_channel_messages(const std::string& channel_id, int limit, const std::string& before, const std::string& after) {
    return pImpl->get_channel_messages(channel_id, limit, before, after);
}

nlohmann::json DiscordClient::send_embed(const std::string& channel_id, const nlohmann::json& embed) {
    return pImpl->send_embed(channel_id, embed);
}

void DiscordClient::delete_message(const std::string& channel_id, const std::string& message_id) {
    pImpl->delete_message(channel_id, message_id);
}

void DiscordClient::edit_message(const std::string& channel_id, const std::string& message_id, const std::string& new_content) {
    pImpl->edit_message(channel_id, message_id, new_content);
}

void DiscordClient::add_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji) {
    pImpl->add_reaction(channel_id, message_id, emoji);
}

void DiscordClient::remove_reaction(const std::string& channel_id, const std::string& message_id, const std::string& emoji, const std::string& user_id) {
    pImpl->remove_reaction(channel_id, message_id, emoji, user_id);
}

void DiscordClient::edit_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id, const nlohmann::json& message) {
    pImpl->edit_followup_message(application_id, interaction_token, message_id, message);
}

void DiscordClient::delete_followup_message(const std::string& application_id, const std::string& interaction_token, const std::string& message_id) {
    pImpl->delete_followup_message(application_id, interaction_token, message_id);
}

void DiscordClient::create_role(const std::string& guild_id, const nlohmann::json& role_data) {
    pImpl->create_role(guild_id, role_data);
}

void DiscordClient::delete_role(const std::string& guild_id, const std::string& role_id) {
    pImpl->delete_role(guild_id, role_id);
}

void DiscordClient::add_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
    pImpl->add_role(guild_id, user_id, role_id);
}

void DiscordClient::remove_role(const std::string& guild_id, const std::string& user_id, const std::string& role_id) {
    pImpl->remove_role(guild_id, user_id, role_id);
}

nlohmann::json DiscordClient::create_text_channel(const std::string& guild_id, const std::string& name, const std::string& parent_id, int position) {
    return pImpl->create_text_channel(guild_id, name, parent_id, position);
}

nlohmann::json DiscordClient::create_voice_channel(const std::string& guild_id, const std::string& name, int bitrate, int user_limit) {
    return pImpl->create_voice_channel(guild_id, name, bitrate, user_limit);
}

void DiscordClient::delete_channel(const std::string& channel_id) {
    pImpl->delete_channel(channel_id);
}

void DiscordClient::edit_channel(const std::string& channel_id, const nlohmann::json& data) {
    pImpl->edit_channel(channel_id, data);
}

void DiscordClient::set_token(const std::string& token) {
    pImpl->set_token(token);
}

const std::string& DiscordClient::get_token() const {
    return pImpl->get_token();
}

void DiscordClient::set_intents(int intents) {
    pImpl->set_intents(intents);
}

int DiscordClient::get_intents() const {
    return pImpl->get_intents();
}

bool DiscordClient::is_connected() const {
    return pImpl->is_connected();
}

bool DiscordClient::is_ready() const {
    return pImpl->is_ready();
}

}