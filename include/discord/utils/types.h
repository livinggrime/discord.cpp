#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <optional>
#include <variant>
#include <nlohmann/json.hpp>

namespace discord {

// Result type for better error handling
template<typename T>
class Result {
private:
    std::variant<T, std::string> value_;
    
public:
    Result(T value) : value_(std::move(value)) {}
    Result(const std::string& error) : value_(error) {}
    
    bool is_success() const {
        return std::holds_alternative<T>(value_);
    }
    
    bool is_error() const {
        return std::holds_alternative<std::string>(value_);
    }
    
    const T& value() const {
        if (is_error()) {
            throw std::runtime_error("Attempted to get value from error result");
        }
        return std::get<T>(value_);
    }
    
    const std::string& error() const {
        if (is_success()) {
            throw std::runtime_error("Attempted to get error from success result");
        }
        return std::get<std::string>(value_);
    }
    
    T value_or(T&& default_value) const {
        return is_success() ? value() : std::move(default_value);
    }
    
    template<typename F>
    Result<T> map(F&& func) const {
        if (is_success()) {
            try {
                return Result<T>(func(value()));
            } catch (const std::exception& e) {
                return Result<T>(e.what());
            }
        }
        return *this;
    }
};

// Specialization for void
template<>
class Result<void> {
private:
    std::optional<std::string> error_;
    
public:
    Result() : error_(std::nullopt) {}
    Result(const std::string& error) : error_(error) {}
    
    bool is_success() const { return !error_.has_value(); }
    bool is_error() const { return error_.has_value(); }
    
    const std::string& error() const {
        if (is_success()) {
            throw std::runtime_error("Attempted to get error from success result");
        }
        return *error_;
    }
};

enum class GatewayIntent {
    GUILDS = 1 << 0,
    GUILD_MEMBERS = 1 << 1,
    GUILD_BANS = 1 << 2,
    GUILD_EMOJIS_AND_STICKERS = 1 << 3,
    GUILD_INTEGRATIONS = 1 << 4,
    GUILD_WEBHOOKS = 1 << 5,
    GUILD_INVITES = 1 << 6,
    GUILD_VOICE_STATES = 1 << 7,
    GUILD_PRESENCES = 1 << 8,
    GUILD_MESSAGES = 1 << 9,
    GUILD_MESSAGE_REACTIONS = 1 << 10,
    GUILD_MESSAGE_TYPING = 1 << 11,
    DIRECT_MESSAGES = 1 << 12,
    DIRECT_MESSAGE_REACTIONS = 1 << 13,
    DIRECT_MESSAGE_TYPING = 1 << 14,
    MESSAGE_CONTENT = 1 << 15,
    GUILD_SCHEDULED_EVENTS = 1 << 16,
    AUTO_MODERATION_CONFIGURATION = 1 << 20,
    AUTO_MODERATION_EXECUTION = 1 << 21
};

// Permission flags
enum class Permission : uint64_t {
    CREATE_INSTANT_INVITE = 0x0000000001,
    KICK_MEMBERS = 0x0000000002,
    BAN_MEMBERS = 0x0000000004,
    ADMINISTRATOR = 0x0000000008,
    MANAGE_CHANNELS = 0x0000000010,
    MANAGE_GUILD = 0x0000000020,
    ADD_REACTIONS = 0x0000000040,
    VIEW_AUDIT_LOG = 0x0000000080,
    PRIORITY_SPEAKER = 0x0000000100,
    STREAM = 0x0000000200,
    VIEW_CHANNEL = 0x0000000400,
    SEND_MESSAGES = 0x0000000800,
    SEND_TTS_MESSAGES = 0x0000001000,
    MANAGE_MESSAGES = 0x0000002000,
    EMBED_LINKS = 0x0000004000,
    ATTACH_FILES = 0x0000008000,
    READ_MESSAGE_HISTORY = 0x0000010000,
    MENTION_EVERYONE = 0x0000020000,
    USE_EXTERNAL_EMOJIS = 0x0000040000,
    VIEW_GUILD_INSIGHTS = 0x0000080000,
    CONNECT = 0x0000100000,
    SPEAK = 0x0000200000,
    MUTE_MEMBERS = 0x0000400000,
    DEAFEN_MEMBERS = 0x0000800000,
    MOVE_MEMBERS = 0x0001000000,
    USE_VAD = 0x0002000000,
    CHANGE_NICKNAME = 0x0004000000,
    MANAGE_NICKNAMES = 0x0008000000,
    MANAGE_ROLES = 0x0010000000,
    MANAGE_WEBHOOKS = 0x0020000000,
    MANAGE_EMOJIS_AND_STICKERS = 0x0040000000,
    USE_APPLICATION_COMMANDS = 0x0080000000,
    REQUEST_TO_SPEAK = 0x0100000000,
    MANAGE_EVENTS = 0x0200000000,
    MANAGE_THREADS = 0x0400000000,
    CREATE_PUBLIC_THREADS = 0x0800000000,
    CREATE_PRIVATE_THREADS = 0x1000000000,
    USE_EXTERNAL_STICKERS = 0x2000000000,
    SEND_MESSAGES_IN_THREADS = 0x4000000000,
    START_EMBEDDED_ACTIVITIES = 0x8000000000,
    MODERATE_MEMBERS = 0x10000000000
};

// Embed structures (defined before use)
struct EmbedFooter {
    std::string text;
    std::optional<std::string> icon_url;
    std::optional<std::string> proxy_icon_url;
};

struct EmbedImage {
    std::optional<std::string> url;
    std::optional<std::string> proxy_url;
    std::optional<int> height;
    std::optional<int> width;
};

struct EmbedThumbnail {
    std::optional<std::string> url;
    std::optional<std::string> proxy_url;
    std::optional<int> height;
    std::optional<int> width;
};

struct EmbedVideo {
    std::optional<std::string> url;
    std::optional<int> height;
    std::optional<int> width;
};

struct EmbedProvider {
    std::optional<std::string> name;
    std::optional<std::string> url;
};

struct EmbedAuthor {
    std::optional<std::string> name;
    std::optional<std::string> url;
    std::optional<std::string> icon_url;
    std::optional<std::string> proxy_icon_url;
};

struct EmbedField {
    std::string name;
    std::string value;
    bool is_inline;
};

// Channel types
enum class ChannelType {
    GUILD_TEXT = 0,
    DM = 1,
    GUILD_VOICE = 2,
    GROUP_DM = 3,
    GUILD_CATEGORY = 4,
    GUILD_ANNOUNCEMENT = 5,
    ANNOUNCEMENT_THREAD = 10,
    PUBLIC_THREAD = 11,
    PRIVATE_THREAD = 12,
    GUILD_STAGE_VOICE = 13,
    GUILD_DIRECTORY = 14,
    GUILD_FORUM = 15
};

// Message types
enum class MessageType {
    DEFAULT = 0,
    RECIPIENT_ADD = 1,
    RECIPIENT_REMOVE = 2,
    CALL = 3,
    CHANNEL_NAME_CHANGE = 4,
    CHANNEL_ICON_CHANGE = 5,
    CHANNEL_PINNED_MESSAGE = 6,
    GUILD_MEMBER_JOIN = 7,
    USER_PREMIUM_GUILD_SUBSCRIPTION = 8,
    USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_1 = 9,
    USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_2 = 10,
    USER_PREMIUM_GUILD_SUBSCRIPTION_TIER_3 = 11,
    CHANNEL_FOLLOW_ADD = 12,
    GUILD_DISCOVERY_DISQUALIFIED = 14,
    GUILD_DISCOVERY_REQUALIFIED = 15,
    GUILD_DISCOVERY_GRACE_PERIOD_INITIAL_WARNING = 16,
    GUILD_DISCOVERY_GRACE_PERIOD_FINAL_WARNING = 17,
    THREAD_CREATED = 18,
    REPLY = 19,
    CHAT_INPUT_COMMAND = 20,
    THREAD_STARTER_MESSAGE = 21,
    GUILD_INVITE_REMINDER = 22,
    CONTEXT_MENU_COMMAND = 23,
    AUTO_MODERATION_ACTION = 24,
    ROLE_SUBSCRIPTION_PURCHASE = 25,
    INTERACTION_PREMIUM_UPSELL = 26,
    STAGE_START = 27,
    STAGE_END = 28,
    STAGE_SPEAKER = 29,
    STAGE_TOPIC = 30,
    GUILD_APPLICATION_PREMIUM_SUBSCRIPTION = 31
};

struct User {
    std::string id;
    std::string username;
    std::string discriminator;
    std::string global_name;
    std::string avatar;
    bool bot;
    bool system;
    bool mfa_enabled;
    std::string locale;
    bool verified;
    std::string email;
    int flags;
    int premium_type;
    int public_flags;
    std::string avatar_decoration;
};

struct Guild {
    std::string id;
    std::string name;
    std::string icon;
    std::string icon_hash;
    std::string splash;
    std::string discovery_splash;
    bool owner;
    std::string owner_id;
    int permissions;
    std::string region;
    std::string afk_channel_id;
    int afk_timeout;
    bool widget_enabled;
    std::string widget_channel_id;
    int verification_level;
    int default_message_notifications;
    int explicit_content_filter;
    std::vector<nlohmann::json> roles;
    std::vector<nlohmann::json> emojis;
    std::vector<std::string> features;
    int mfa_level;
    std::string application_id;
    bool system_channel_flags;
    std::string rules_channel_id;
    int max_members;
    int max_presences;
    std::string vanity_url_code;
    std::string description;
    std::string banner;
    int premium_tier;
    int premium_subscription_count;
    std::string preferred_locale;
    std::string public_updates_channel_id;
    int max_video_channel_users;
    int approximate_member_count;
    int approximate_presence_count;
};

struct Channel {
    std::string id;
    int type;
    std::string guild_id;
    int position;
    std::vector<nlohmann::json> permission_overwrites;
    std::string name;
    std::string topic;
    bool nsfw;
    std::string last_message_id;
    int bitrate;
    int user_limit;
    int rate_limit_per_user;
    std::vector<nlohmann::json> recipients;
    std::string icon;
    std::string owner_id;
    std::string application_id;
    std::string parent_id;
    std::string last_pin_timestamp;
    std::vector<nlohmann::json> messages;
};

struct Message {
    std::string id;
    std::string channel_id;
    std::optional<std::string> guild_id;
    User author;
    std::optional<std::string> member;
    std::string content;
    std::chrono::system_clock::time_point timestamp;
    std::optional<std::chrono::system_clock::time_point> edited_timestamp;
    bool tts;
    bool mention_everyone;
    std::vector<User> mentions;
    std::vector<std::string> mention_roles;
    std::vector<std::string> mention_channels;
    std::vector<nlohmann::json> attachments;
    std::vector<nlohmann::json> embeds;
    std::vector<nlohmann::json> reactions;
    std::string nonce;
    bool pinned;
    std::optional<int> webhook_id;
    MessageType type;
    std::optional<std::vector<nlohmann::json>> components;
    std::optional<std::string> message_reference;
    std::optional<int> flags;
    std::optional<nlohmann::json> interaction;
    std::optional<std::string> thread;
    std::optional<nlohmann::json> application;
    std::optional<nlohmann::json> application_id;
    std::optional<nlohmann::json> activity;
    std::optional<nlohmann::json> sticker_items;
};

struct Role {
    std::string id;
    std::string name;
    int color;
    bool hoist;
    std::string icon;
    std::string unicode_emoji;
    int position;
    std::string permissions;
    bool managed;
    bool mentionable;
    std::vector<std::string> tags;
};

struct Member {
    User user;
    std::string nick;
    std::string avatar;
    std::vector<std::string> roles;
    std::chrono::system_clock::time_point joined_at;
    std::optional<std::chrono::system_clock::time_point> premium_since;
    bool deaf;
    bool mute;
    std::string permissions;
    std::optional<std::chrono::system_clock::time_point> communication_disabled_until;
};

struct Embed {
    std::optional<std::string> title;
    std::optional<std::string> type;
    std::optional<std::string> description;
    std::optional<std::string> url;
    std::optional<std::chrono::system_clock::time_point> timestamp;
    std::optional<int> color;
    std::optional<EmbedFooter> footer;
    std::optional<EmbedImage> image;
    std::optional<EmbedThumbnail> thumbnail;
    std::optional<EmbedVideo> video;
    std::optional<EmbedProvider> provider;
    std::optional<EmbedAuthor> author;
    std::vector<EmbedField> fields;
};



void to_json(nlohmann::json& j, const User& user);
void from_json(const nlohmann::json& j, User& user);

void to_json(nlohmann::json& j, const Guild& guild);
void from_json(const nlohmann::json& j, Guild& guild);

void to_json(nlohmann::json& j, const Channel& channel);
void from_json(const nlohmann::json& j, Channel& channel);

}