#include <discord/events/event_handlers.h>
#include <discord/utils/logger.h>
#include <discord/events/event_dispatcher.h>

namespace discord {

EventHandlers::EventHandlers(EventDispatcher* dispatcher) 
    : dispatcher_(dispatcher) {
    LOG_INFO("EventHandlers initialized");
}

EventHandlers::~EventHandlers() {
    clear_all();
    LOG_INFO("EventHandlers destroyed");
}

std::string EventHandlers::register_handler(const std::string& event_name, 
                                       std::function<void(const nlohmann::json&)> callback,
                                       const std::string& handler_id) {
    if (!dispatcher_) {
        LOG_ERROR("EventDispatcher is null");
        return "";
    }
    
    std::string final_handler_id = handler_id;
    if (final_handler_id.empty()) {
        final_handler_id = "handler_" + std::to_string(reinterpret_cast<uintptr_t>(callback.target_type()));
    }
    
    std::string registered_id = dispatcher_->on(event_name, std::move(callback), 0, final_handler_id);
    registered_handler_ids_.push_back(registered_id);
    
    LOG_DEBUG("Registered handler for event: " + event_name + " with ID: " + final_handler_id);
    return final_handler_id;
}

// Message events

std::string EventHandlers::on_message(MessageCallback callback, MessageFilter filter) {
    auto wrapped_callback = [callback](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            callback(event);
        }
    };
    
    return register_handler("MESSAGE_CREATE", std::move(wrapped_callback));
}

std::string EventHandlers::on_message_update(MessageCallback callback, MessageFilter filter) {
    auto wrapped_callback = [callback](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            callback(event);
        }
    };
    
    return register_handler("MESSAGE_UPDATE", std::move(wrapped_callback));
}

std::string EventHandlers::on_message_delete(MessageCallback callback, MessageFilter filter) {
    auto wrapped_callback = [callback](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            callback(event);
        }
    };
    
    return register_handler("MESSAGE_DELETE", std::move(wrapped_callback));
}

std::string EventHandlers::on_message_bulk_delete(MessageCallback callback) {
    return register_handler("MESSAGE_DELETE_BULK", std::move(callback));
}

// Reaction events

std::string EventHandlers::on_reaction_add(ReactionCallback callback, MessageFilter filter) {
    auto wrapped_callback = [callback](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            callback(event);
        }
    };
    
    return register_handler("MESSAGE_REACTION_ADD", std::move(wrapped_callback));
}

std::string EventHandlers::on_reaction_remove(ReactionCallback callback, MessageFilter filter) {
    auto wrapped_callback = [callback](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            callback(event);
        }
    };
    
    return register_handler("MESSAGE_REACTION_REMOVE", std::move(wrapped_callback));
}

std::string EventHandlers::on_reaction_clear(ReactionCallback callback, MessageFilter filter) {
    auto wrapped_callback = [callback](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            callback(event);
        }
    };
    
    return register_handler("MESSAGE_REACTION_REMOVE_ALL", std::move(wrapped_callback));
}

// Guild events

std::string EventHandlers::on_guild_create(GuildCallback callback) {
    return register_handler("GUILD_CREATE", std::move(callback));
}

std::string EventHandlers::on_guild_update(GuildCallback callback) {
    return register_handler("GUILD_UPDATE", std::move(callback));
}

std::string EventHandlers::on_guild_delete(GuildCallback callback) {
    return register_handler("GUILD_DELETE", std::move(callback));
}

// Member events

std::string EventHandlers::on_member_join(MemberCallback callback, MessageFilter filter) {
    auto wrapped_callback = [callback](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            callback(event);
        }
    };
    
    return register_handler("GUILD_MEMBER_ADD", std::move(wrapped_callback));
}

std::string EventHandlers::on_member_remove(MemberCallback callback, MessageFilter filter) {
    auto wrapped_callback = [callback](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            callback(event);
        }
    };
    
    return register_handler("GUILD_MEMBER_REMOVE", std::move(wrapped_callback));
}

std::string EventHandlers::on_member_update(MemberCallback callback) {
    return register_handler("GUILD_MEMBER_UPDATE", std::move(callback));
}

// Channel events

std::string EventHandlers::on_channel_create(ChannelCallback callback) {
    return register_handler("CHANNEL_CREATE", std::move(callback));
}

std::string EventHandlers::on_channel_update(ChannelCallback callback) {
    return register_handler("CHANNEL_UPDATE", std::move(callback));
}

std::string EventHandlers::on_channel_delete(ChannelCallback callback) {
    return register_handler("CHANNEL_DELETE", std::move(callback));
}

// Voice events

std::string EventHandlers::on_voice_state_update(VoiceCallback callback) {
    return register_handler("VOICE_STATE_UPDATE", std::move(callback));
}

// Interaction events

std::string EventHandlers::on_interaction_create(InteractionCallback callback, MessageFilter filter) {
    auto wrapped_callback = [callback](const nlohmann::json& event) {
        if (!filter || filter(event)) {
            callback(event);
        }
    };
    
    return register_handler("INTERACTION_CREATE", std::move(wrapped_callback));
}

void EventHandlers::clear_all() {
    if (!dispatcher_) {
        return;
    }
    
    for (const auto& handler_id : registered_handler_ids_) {
        dispatcher_->off("", handler_id); // Remove all handlers by ID
    }
    
    registered_handler_ids_.clear();
    LOG_INFO("Cleared all event handlers");
}

size_t EventHandlers::get_handler_count() const {
    return registered_handler_ids_.size();
}

std::shared_ptr<EventCollector<nlohmann::json>> EventHandlers::create_message_collector(
        EventFilter filter,
        const CollectorConfig& config) {
    
    if (!dispatcher_) {
        LOG_ERROR("EventDispatcher is null");
        return nullptr;
    }
    
    return dispatcher_->create_collector<nlohmann::json>("MESSAGE_CREATE", filter, config);
}

std::shared_ptr<EventCollector<nlohmann::json>> EventHandlers::create_reaction_collector(
        EventFilter filter,
        const CollectorConfig& config) {
    
    if (!dispatcher_) {
        LOG_ERROR("EventDispatcher is null");
        return nullptr;
    }
    
    return dispatcher_->create_collector<nlohmann::json>("MESSAGE_REACTION_ADD", filter, config);
}

// EventUtils namespace implementations

namespace EventUtils {

std::string extract_message_id(const nlohmann::json& event) {
    if (event.contains("id")) {
        return event["id"];
    }
    return "";
}

std::string extract_channel_id(const nlohmann::json& event) {
    if (event.contains("channel_id")) {
        return event["channel_id"];
    }
    return "";
}

std::string extract_guild_id(const nlohmann::json& event) {
    if (event.contains("guild_id")) {
        return event["guild_id"];
    }
    return "";
}

std::string extract_user_id(const nlohmann::json& event) {
    if (event.contains("author") && event["author"].contains("id")) {
        return event["author"]["id"];
    }
    return "";
}

bool is_bot_message(const nlohmann::json& event) {
    if (event.contains("author") && event["author"].contains("bot")) {
        return event["author"]["bot"];
    }
    return false;
}

bool mentions_user(const nlohmann::json& event, const std::string& user_id) {
    if (!event.contains("mentions")) {
        return false;
    }
    
    for (const auto& mention : event["mentions"]) {
        if (mention.contains("id") && mention["id"] == user_id) {
            return true;
        }
    }
    
    return false;
}

bool is_in_channel(const nlohmann::json& event, const std::string& channel_id) {
    return extract_channel_id(event) == channel_id;
}

bool is_in_guild(const nlohmann::json& event, const std::string& guild_id) {
    return extract_guild_id(event) == guild_id;
}

} // namespace EventUtils

} // namespace discord