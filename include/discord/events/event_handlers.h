#pragma once

#include <string>
#include <functional>
#include <unordered_map>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>
#include "event_dispatcher.h"

namespace discord {

/**
 * @brief Specific event handlers for Discord events
 * 
 * Provides typed handlers for common Discord events with proper
 * data structures and validation.
 * 
 * @todo TODO.md: Implement comprehensive event system with collectors and filters
 */
class EventHandlers {
public:
    // Message event handlers
    using MessageCallback = std::function<void(const nlohmann::json&)>;
    using MessageFilter = std::function<bool(const nlohmann::json&)>;
    
    // Reaction event handlers
    using ReactionCallback = std::function<void(const nlohmann::json&)>;
    
    // Guild event handlers
    using GuildCallback = std::function<void(const nlohmann::json&)>;
    
    // Member event handlers
    using MemberCallback = std::function<void(const nlohmann::json&)>;
    
    // Channel event handlers
    using ChannelCallback = std::function<void(const nlohmann::json&)>;
    
    // Voice event handlers
    using VoiceCallback = std::function<void(const nlohmann::json&)>;
    
    // Interaction event handlers
    using InteractionCallback = std::function<void(const nlohmann::json&)>;

private:
    EventDispatcher* dispatcher_;
    
    // Handler storage for cleanup
    std::vector<std::string> registered_handler_ids_;

    /**
     * @brief Register handler and store ID for cleanup
     * @param event_name Event name
     * @param callback Handler function
     * @param handler_id Handler ID
     * @return Registered handler ID
     */
    std::string register_handler(const std::string& event_name, 
                              std::function<void(const nlohmann::json&)> callback,
                              const std::string& handler_id = "");

public:
    /**
     * @brief Construct EventHandlers
     * @param dispatcher Event dispatcher to use
     */
    explicit EventHandlers(EventDispatcher* dispatcher);

    /**
     * @brief Destructor - cleans up all registered handlers
     */
    ~EventHandlers();

    // Message events
    /**
     * @brief Handle message creation
     * @param callback Handler function
     * @param filter Optional filter function
     * @return Handler ID
     */
    std::string on_message(MessageCallback callback, MessageFilter filter = nullptr);
    
    /**
     * @brief Handle message update
     * @param callback Handler function
     * @param filter Optional filter function
     * @return Handler ID
     */
    std::string on_message_update(MessageCallback callback, MessageFilter filter = nullptr);
    
    /**
     * @brief Handle message deletion
     * @param callback Handler function
     * @param filter Optional filter function
     * @return Handler ID
     */
    std::string on_message_delete(MessageCallback callback, MessageFilter filter = nullptr);
    
    /**
     * @brief Handle message bulk delete
     * @param callback Handler function
     * @return Handler ID
     */
    std::string on_message_bulk_delete(MessageCallback callback);

    // Reaction events
    /**
     * @brief Handle reaction addition
     * @param callback Handler function
     * @param filter Optional filter function
     * @return Handler ID
     */
    std::string on_reaction_add(ReactionCallback callback, MessageFilter filter = nullptr);
    
    /**
     * @brief Handle reaction removal
     * @param callback Handler function
     * @param filter Optional filter function
     * @return Handler ID
     */
    std::string on_reaction_remove(ReactionCallback callback, MessageFilter filter = nullptr);
    
    /**
     * @brief Handle reaction clear
     * @param callback Handler function
     * @param filter Optional filter function
     * @return Handler ID
     */
    std::string on_reaction_clear(ReactionCallback callback, MessageFilter filter = nullptr);

    // Guild events
    /**
     * @brief Handle guild creation
     * @param callback Handler function
     * @return Handler ID
     */
    std::string on_guild_create(GuildCallback callback);
    
    /**
     * @brief Handle guild update
     * @param callback Handler function
     * @return Handler ID
     */
    std::string on_guild_update(GuildCallback callback);
    
    /**
     * @brief Handle guild deletion
     * @param callback Handler function
     * @return Handler ID
     */
    std::string on_guild_delete(GuildCallback callback);

    // Member events
    /**
     * @brief Handle member join
     * @param callback Handler function
     * @param filter Optional filter function
     * @return Handler ID
     */
    std::string on_member_join(MemberCallback callback, MessageFilter filter = nullptr);
    
    /**
     * @brief Handle member leave
     * @param callback Handler function
     * @param filter Optional filter function
     * @return Handler ID
     */
    std::string on_member_remove(MemberCallback callback, MessageFilter filter = nullptr);
    
    /**
     * @brief Handle member update
     * @param callback Handler function
     * @return Handler ID
     */
    std::string on_member_update(MemberCallback callback);

    // Channel events
    /**
     * @brief Handle channel creation
     * @param callback Handler function
     * @return Handler ID
     */
    std::string on_channel_create(ChannelCallback callback);
    
    /**
     * @brief Handle channel update
     * @param callback Handler function
     * @return Handler ID
     */
    std::string on_channel_update(ChannelCallback callback);
    
    /**
     * @brief Handle channel deletion
     * @param callback Handler function
     * @return Handler ID
     */
    std::string on_channel_delete(ChannelCallback callback);

    // Voice events
    /**
     * @brief Handle voice state update
     * @param callback Handler function
     * @return Handler ID
     */
    std::string on_voice_state_update(VoiceCallback callback);

    // Interaction events
    /**
     * @brief Handle interaction creation
     * @param callback Handler function
     * @param filter Optional filter function
     * @return Handler ID
     */
    std::string on_interaction_create(InteractionCallback callback, MessageFilter filter = nullptr);

    /**
     * @brief Remove all registered handlers
     */
    void clear_all();

    /**
     * @brief Get number of registered handlers
     * @return Total handler count
     */
    size_t get_handler_count() const;

    /**
     * @brief Create message collector
     * @param filter Filter function
     * @param config Collector configuration
     * @return Shared pointer to message collector
     */
    std::shared_ptr<EventCollector<nlohmann::json>> create_message_collector(
        EventFilter filter = nullptr,
        const CollectorConfig& config = CollectorConfig{});

    /**
     * @brief Create reaction collector
     * @param filter Filter function
     * @param config Collector configuration
     * @return Shared pointer to reaction collector
     */
    std::shared_ptr<EventCollector<nlohmann::json>> create_reaction_collector(
        EventFilter filter = nullptr,
        const CollectorConfig& config = CollectorConfig{});
};

/**
 * @brief Event handler utilities
 */
namespace EventUtils {
    /**
     * @brief Extract message ID from event
     * @param event Event data
     * @return Message ID or empty string
     */
    std::string extract_message_id(const nlohmann::json& event);
    
    /**
     * @brief Extract channel ID from event
     * @param event Event data
     * @return Channel ID or empty string
     */
    std::string extract_channel_id(const nlohmann::json& event);
    
    /**
     * @brief Extract guild ID from event
     * @param event Event data
     * @return Guild ID or empty string
     */
    std::string extract_guild_id(const nlohmann::json& event);
    
    /**
     * @brief Extract user ID from event
     * @param event Event data
     * @return User ID or empty string
     */
    std::string extract_user_id(const nlohmann::json& event);
    
    /**
     * @brief Check if event is from a bot
     * @param event Event data
     * @return True if author is a bot
     */
    bool is_bot_message(const nlohmann::json& event);
    
    /**
     * @brief Check if message mentions user
     * @param event Message event
     * @param user_id User ID to check
     * @return True if user is mentioned
     */
    bool mentions_user(const nlohmann::json& event, const std::string& user_id);
    
    /**
     * @brief Check if message is in specified channel
     * @param event Message event
     * @param channel_id Channel ID to check
     * @return True if message is in channel
     */
    bool is_in_channel(const nlohmann::json& event, const std::string& channel_id);
    
    /**
     * @brief Check if message is in specified guild
     * @param event Message event
     * @param guild_id Guild ID to check
     * @return True if message is in guild
     */
    bool is_in_guild(const nlohmann::json& event, const std::string& guild_id);
}

} // namespace discord