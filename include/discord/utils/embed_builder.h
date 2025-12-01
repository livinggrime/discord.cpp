#pragma once

#include "types.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional>

namespace discord {

class EmbedBuilder {
private:
    nlohmann::json embed_;
    
public:
    EmbedBuilder();
    ~EmbedBuilder() = default;
    
    EmbedBuilder& title(const std::string& title);
    EmbedBuilder& description(const std::string& description);
    EmbedBuilder& url(const std::string& url);
    EmbedBuilder& color(int color);
    EmbedBuilder& timestamp(const std::chrono::system_clock::time_point& timestamp);
    EmbedBuilder& footer(const std::string& text, const std::string& icon_url = std::string());
    EmbedBuilder& thumbnail(const std::string& url, int height = 0, int width = 0);
    EmbedBuilder& image(const std::string& url, int height = 0, int width = 0);
    EmbedBuilder& author(const std::string& name, const std::string& url = std::string(), const std::string& icon_url = std::string());
    EmbedBuilder& field(const std::string& name, const std::string& value, bool is_inline = false);
    
    nlohmann::json build() const;
    
    // Static helper methods
    static int color_from_rgb(int r, int g, int b);
    static int color_from_hex(const std::string& hex);
    
    // Predefined colors
    static const int DEFAULT_COLOR = 0x000000;
    static const int AQUA = 0x1ABC9C;
    static const int GREEN = 0x2ECC71;
    static const int BLUE = 0x3498DB;
    static const int PURPLE = 0x9B59B6;
    static const int LUMINOUS_VIVID_PINK = 0xE91E63;
    static const int GOLD = 0xF1C40F;
    static const int ORANGE = 0xE67E22;
    static const int RED = 0xE74C3C;
    static const int GREY = 0x95A5A6;
    static const int NAVY = 0x34495E;
    static const int DARK_AQUA = 0x11806A;
    static const int DARK_GREEN = 0x1F8B4C;
    static const int DARK_BLUE = 0x206694;
    static const int DARK_PURPLE = 0x71368A;
    static const int DARK_VIVID_PINK = 0xAD1457;
    static const int DARK_GOLD = 0xC27C0E;
    static const int DARK_ORANGE = 0xA84300;
    static const int DARK_RED = 0x992D22;
    static const int DARK_GREY = 0x979C9F;
    static const int DARKER_GREY = 0x7F8C8D;
    static const int LIGHT_GREY = 0xBCC0C0;
    static const int DARK_NAVY = 0x2C3E50;
    static const int BLURPLE = 0x5865F2;
    static const int GREYPLE = 0x99AAB5;
    static const int DARK_BUT_NOT_BLACK = 0x2C2F33;
    static const int NOT_QUITE_BLACK = 0x23272A;
};

} // namespace discord