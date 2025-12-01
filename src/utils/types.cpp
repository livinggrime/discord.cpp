#include <discord/utils/types.h>

namespace discord {

void to_json(nlohmann::json& j, const User& user) {
    j = nlohmann::json{
        {"id", user.id},
        {"username", user.username},
        {"discriminator", user.discriminator},
        {"global_name", user.global_name},
        {"avatar", user.avatar},
        {"bot", user.bot},
        {"system", user.system},
        {"mfa_enabled", user.mfa_enabled},
        {"locale", user.locale},
        {"verified", user.verified},
        {"email", user.email},
        {"flags", user.flags},
        {"premium_type", user.premium_type},
        {"public_flags", user.public_flags},
        {"avatar_decoration", user.avatar_decoration}
    };
}

void from_json(const nlohmann::json& j, User& user) {
    j.at("id").get_to(user.id);
    j.at("username").get_to(user.username);
    j.at("discriminator").get_to(user.discriminator);
    if (j.contains("global_name")) j.at("global_name").get_to(user.global_name);
    if (j.contains("avatar")) j.at("avatar").get_to(user.avatar);
    if (j.contains("bot")) j.at("bot").get_to(user.bot);
    if (j.contains("system")) j.at("system").get_to(user.system);
    if (j.contains("mfa_enabled")) j.at("mfa_enabled").get_to(user.mfa_enabled);
    if (j.contains("locale")) j.at("locale").get_to(user.locale);
    if (j.contains("verified")) j.at("verified").get_to(user.verified);
    if (j.contains("email")) j.at("email").get_to(user.email);
    if (j.contains("flags")) j.at("flags").get_to(user.flags);
    if (j.contains("premium_type")) j.at("premium_type").get_to(user.premium_type);
    if (j.contains("public_flags")) j.at("public_flags").get_to(user.public_flags);
    if (j.contains("avatar_decoration")) j.at("avatar_decoration").get_to(user.avatar_decoration);
}

void to_json(nlohmann::json& j, const Guild& guild) {
    j = nlohmann::json{
        {"id", guild.id},
        {"name", guild.name},
        {"icon", guild.icon},
        {"icon_hash", guild.icon_hash},
        {"splash", guild.splash},
        {"discovery_splash", guild.discovery_splash},
        {"owner", guild.owner},
        {"owner_id", guild.owner_id},
        {"permissions", guild.permissions},
        {"region", guild.region},
        {"afk_channel_id", guild.afk_channel_id},
        {"afk_timeout", guild.afk_timeout},
        {"widget_enabled", guild.widget_enabled},
        {"widget_channel_id", guild.widget_channel_id},
        {"verification_level", guild.verification_level},
        {"default_message_notifications", guild.default_message_notifications},
        {"explicit_content_filter", guild.explicit_content_filter},
        {"roles", guild.roles},
        {"emojis", guild.emojis},
        {"features", guild.features},
        {"mfa_level", guild.mfa_level},
        {"application_id", guild.application_id},
        {"system_channel_flags", guild.system_channel_flags},
        {"rules_channel_id", guild.rules_channel_id},
        {"max_members", guild.max_members},
        {"max_presences", guild.max_presences},
        {"vanity_url_code", guild.vanity_url_code},
        {"description", guild.description},
        {"banner", guild.banner},
        {"premium_tier", guild.premium_tier},
        {"premium_subscription_count", guild.premium_subscription_count},
        {"preferred_locale", guild.preferred_locale},
        {"public_updates_channel_id", guild.public_updates_channel_id},
        {"max_video_channel_users", guild.max_video_channel_users},
        {"approximate_member_count", guild.approximate_member_count},
        {"approximate_presence_count", guild.approximate_presence_count}
    };
}

void from_json(const nlohmann::json& j, Guild& guild) {
    j.at("id").get_to(guild.id);
    j.at("name").get_to(guild.name);
    if (j.contains("icon")) j.at("icon").get_to(guild.icon);
    if (j.contains("icon_hash")) j.at("icon_hash").get_to(guild.icon_hash);
    if (j.contains("splash")) j.at("splash").get_to(guild.splash);
    if (j.contains("discovery_splash")) j.at("discovery_splash").get_to(guild.discovery_splash);
    if (j.contains("owner")) j.at("owner").get_to(guild.owner);
    if (j.contains("owner_id")) j.at("owner_id").get_to(guild.owner_id);
    if (j.contains("permissions")) j.at("permissions").get_to(guild.permissions);
    if (j.contains("region")) j.at("region").get_to(guild.region);
    if (j.contains("afk_channel_id")) j.at("afk_channel_id").get_to(guild.afk_channel_id);
    if (j.contains("afk_timeout")) j.at("afk_timeout").get_to(guild.afk_timeout);
    if (j.contains("widget_enabled")) j.at("widget_enabled").get_to(guild.widget_enabled);
    if (j.contains("widget_channel_id")) j.at("widget_channel_id").get_to(guild.widget_channel_id);
    if (j.contains("verification_level")) j.at("verification_level").get_to(guild.verification_level);
    if (j.contains("default_message_notifications")) j.at("default_message_notifications").get_to(guild.default_message_notifications);
    if (j.contains("explicit_content_filter")) j.at("explicit_content_filter").get_to(guild.explicit_content_filter);
    if (j.contains("roles")) j.at("roles").get_to(guild.roles);
    if (j.contains("emojis")) j.at("emojis").get_to(guild.emojis);
    if (j.contains("features")) j.at("features").get_to(guild.features);
    if (j.contains("mfa_level")) j.at("mfa_level").get_to(guild.mfa_level);
    if (j.contains("application_id")) j.at("application_id").get_to(guild.application_id);
    if (j.contains("system_channel_flags")) j.at("system_channel_flags").get_to(guild.system_channel_flags);
    if (j.contains("rules_channel_id")) j.at("rules_channel_id").get_to(guild.rules_channel_id);
    if (j.contains("max_members")) j.at("max_members").get_to(guild.max_members);
    if (j.contains("max_presences")) j.at("max_presences").get_to(guild.max_presences);
    if (j.contains("vanity_url_code")) j.at("vanity_url_code").get_to(guild.vanity_url_code);
    if (j.contains("description")) j.at("description").get_to(guild.description);
    if (j.contains("banner")) j.at("banner").get_to(guild.banner);
    if (j.contains("premium_tier")) j.at("premium_tier").get_to(guild.premium_tier);
    if (j.contains("premium_subscription_count")) j.at("premium_subscription_count").get_to(guild.premium_subscription_count);
    if (j.contains("preferred_locale")) j.at("preferred_locale").get_to(guild.preferred_locale);
    if (j.contains("public_updates_channel_id")) j.at("public_updates_channel_id").get_to(guild.public_updates_channel_id);
    if (j.contains("max_video_channel_users")) j.at("max_video_channel_users").get_to(guild.max_video_channel_users);
    if (j.contains("approximate_member_count")) j.at("approximate_member_count").get_to(guild.approximate_member_count);
    if (j.contains("approximate_presence_count")) j.at("approximate_presence_count").get_to(guild.approximate_presence_count);
}

void to_json(nlohmann::json& j, const Channel& channel) {
    j = nlohmann::json{
        {"id", channel.id},
        {"type", channel.type},
        {"guild_id", channel.guild_id},
        {"position", channel.position},
        {"permission_overwrites", channel.permission_overwrites},
        {"name", channel.name},
        {"topic", channel.topic},
        {"nsfw", channel.nsfw},
        {"last_message_id", channel.last_message_id},
        {"bitrate", channel.bitrate},
        {"user_limit", channel.user_limit},
        {"rate_limit_per_user", channel.rate_limit_per_user},
        {"recipients", channel.recipients},
        {"icon", channel.icon},
        {"owner_id", channel.owner_id},
        {"application_id", channel.application_id},
        {"parent_id", channel.parent_id},
        {"last_pin_timestamp", channel.last_pin_timestamp},
        {"messages", channel.messages}
    };
}

void from_json(const nlohmann::json& j, Channel& channel) {
    j.at("id").get_to(channel.id);
    j.at("type").get_to(channel.type);
    if (j.contains("guild_id")) j.at("guild_id").get_to(channel.guild_id);
    if (j.contains("position")) j.at("position").get_to(channel.position);
    if (j.contains("permission_overwrites")) j.at("permission_overwrites").get_to(channel.permission_overwrites);
    if (j.contains("name")) j.at("name").get_to(channel.name);
    if (j.contains("topic")) j.at("topic").get_to(channel.topic);
    if (j.contains("nsfw")) j.at("nsfw").get_to(channel.nsfw);
    if (j.contains("last_message_id")) j.at("last_message_id").get_to(channel.last_message_id);
    if (j.contains("bitrate")) j.at("bitrate").get_to(channel.bitrate);
    if (j.contains("user_limit")) j.at("user_limit").get_to(channel.user_limit);
    if (j.contains("rate_limit_per_user")) j.at("rate_limit_per_user").get_to(channel.rate_limit_per_user);
    if (j.contains("recipients")) j.at("recipients").get_to(channel.recipients);
    if (j.contains("icon")) j.at("icon").get_to(channel.icon);
    if (j.contains("owner_id")) j.at("owner_id").get_to(channel.owner_id);
    if (j.contains("application_id")) j.at("application_id").get_to(channel.application_id);
    if (j.contains("parent_id")) j.at("parent_id").get_to(channel.parent_id);
    if (j.contains("last_pin_timestamp")) j.at("last_pin_timestamp").get_to(channel.last_pin_timestamp);
    if (j.contains("messages")) j.at("messages").get_to(channel.messages);
}

}