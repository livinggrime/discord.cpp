#include <discord/utils/embed_builder.h>
#include <sstream>
#include <iomanip>

namespace discord {

EmbedBuilder::EmbedBuilder() {
    embed_ = nlohmann::json::object();
}

EmbedBuilder& EmbedBuilder::title(const std::string& title) {
    embed_["title"] = title;
    return *this;
}

EmbedBuilder& EmbedBuilder::description(const std::string& description) {
    embed_["description"] = description;
    return *this;
}

EmbedBuilder& EmbedBuilder::url(const std::string& url) {
    embed_["url"] = url;
    return *this;
}

EmbedBuilder& EmbedBuilder::color(int color) {
    embed_["color"] = color;
    return *this;
}

EmbedBuilder& EmbedBuilder::timestamp(const std::chrono::system_clock::time_point& timestamp) {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    embed_["timestamp"] = std::put_time(std::gmtime(&time_t), "%Y-%m-%dT%H:%M:%S.000Z");
    return *this;
}

EmbedBuilder& EmbedBuilder::footer(const std::string& text, const std::string& icon_url) {
    nlohmann::json footer = nlohmann::json::object();
    footer["text"] = text;
    if (!icon_url.empty()) {
        footer["icon_url"] = icon_url;
    }
    embed_["footer"] = footer;
    return *this;
}

EmbedBuilder& EmbedBuilder::thumbnail(const std::string& url, int height, int width) {
    nlohmann::json thumbnail = nlohmann::json::object();
    thumbnail["url"] = url;
    if (height > 0) thumbnail["height"] = height;
    if (width > 0) thumbnail["width"] = width;
    embed_["thumbnail"] = thumbnail;
    return *this;
}

EmbedBuilder& EmbedBuilder::image(const std::string& url, int height, int width) {
    nlohmann::json image = nlohmann::json::object();
    image["url"] = url;
    if (height > 0) image["height"] = height;
    if (width > 0) image["width"] = width;
    embed_["image"] = image;
    return *this;
}

EmbedBuilder& EmbedBuilder::author(const std::string& name, const std::string& url, const std::string& icon_url) {
    nlohmann::json author = nlohmann::json::object();
    author["name"] = name;
    if (!url.empty()) author["url"] = url;
    if (!icon_url.empty()) author["icon_url"] = icon_url;
    embed_["author"] = author;
    return *this;
}

EmbedBuilder& EmbedBuilder::field(const std::string& name, const std::string& value, bool is_inline) {
    if (!embed_.contains("fields")) {
        embed_["fields"] = nlohmann::json::array();
    }
    
    nlohmann::json field = nlohmann::json::object();
    field["name"] = name;
    field["value"] = value;
    field["inline"] = is_inline;
    
    embed_["fields"].push_back(field);
    return *this;
}

nlohmann::json EmbedBuilder::build() const {
    return embed_;
}

int EmbedBuilder::color_from_rgb(int r, int g, int b) {
    return (r << 16) | (g << 8) | b;
}

int EmbedBuilder::color_from_hex(const std::string& hex) {
    std::string hex_color = hex;
    if (hex_color[0] == '#') {
        hex_color = hex_color.substr(1);
    }
    
    std::stringstream ss;
    ss << std::hex << hex_color;
    int color;
    ss >> color;
    
    return color;
}

} // namespace discord