/**
 * @file button.cpp
 * @brief Button component implementation
 */

#include "../../include/discord/components/button.h"
#include <stdexcept>
#include <memory>
#include <nlohmann/json.hpp>

namespace discord::components {

    discord::components::Button::Button(const std::string& label,
                   discord::components::ButtonStyle style,
                   const std::string& custom_id,
                   const std::optional<std::string>& emoji,
                   bool disabled)
        : InteractiveComponent(custom_id, disabled),
          style_(style),
          label_(label),
          emoji_(emoji) {
        
        if (label_.empty()) {
            throw std::invalid_argument("Button label cannot be empty");
        }
        
        if (label_.length() > 80) {
            throw std::invalid_argument("Button label cannot exceed 80 characters");
        }
        
        if (style_ == discord::components::ButtonStyle::LINK && custom_id_.empty() && !url_.has_value()) {
            throw std::invalid_argument("Link buttons must have a URL");
        }
        
        if (style_ != discord::components::ButtonStyle::LINK && !custom_id_.empty()) {
            throw std::invalid_argument("Non-link buttons must have a custom_id");
        }
    }

    discord::components::Button::Button(const std::string& label,
                   const std::string& url,
                   const std::optional<std::string>& emoji,
                   bool disabled)
        : InteractiveComponent("", disabled),
          style_(discord::components::ButtonStyle::LINK),
          label_(label),
          url_(url),
          emoji_(emoji) {
        
        if (label_.empty()) {
            throw std::invalid_argument("Button label cannot be empty");
        }
        
        if (label_.length() > 80) {
            throw std::invalid_argument("Button label cannot exceed 80 characters");
        }
        
        if (url_.value_or("").empty()) {
            throw std::invalid_argument("Link button URL cannot be empty");
        }
    }

    nlohmann::json discord::components::Button::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(discord::components::ComponentType::BUTTON);
        json["style"] = static_cast<int>(style_);
        json["label"] = label_;
        
        if (style_ != discord::components::ButtonStyle::LINK) {
            json["custom_id"] = custom_id_;
        } else {
            json["url"] = url_.value();
        }
        
        if (emoji_.has_value()) {
            json["emoji"] = {{"name", emoji_.value()}};
        }
        
        if (disabled_) {
            json["disabled"] = true;
        }
        
        return json;
    }

    bool discord::components::Button::validate() const {
        if (label_.empty() || label_.length() > 80) {
            return false;
        }
        
        if (style_ == discord::components::ButtonStyle::LINK) {
            return url_.has_value() && !url_.value().empty();
        } else {
            return !custom_id_.empty() && custom_id_.length() <= 100;
        }
    }

    std::unique_ptr<discord::components::IComponent> discord::components::Button::clone() const {
        if (style_ == discord::components::ButtonStyle::LINK) {
            return std::make_unique<Button>(label_, url_.value(), emoji_, disabled_);
        } else {
            return std::make_unique<Button>(label_, style_, custom_id_, emoji_, disabled_);
        }
    }

    // Static factory methods
    std::unique_ptr<discord::components::Button> discord::components::Button::primary(const std::string& label,
                                            const std::string& custom_id,
                                            const std::optional<std::string>& emoji) {
        return std::make_unique<Button>(label, discord::components::ButtonStyle::PRIMARY, custom_id, emoji);
    }

    std::unique_ptr<discord::components::Button> discord::components::Button::secondary(const std::string& label,
                                              const std::string& custom_id,
                                              const std::optional<std::string>& emoji) {
        return std::make_unique<Button>(label, discord::components::ButtonStyle::SECONDARY, custom_id, emoji);
    }

    std::unique_ptr<discord::components::Button> discord::components::Button::success(const std::string& label,
                                            const std::string& custom_id,
                                            const std::optional<std::string>& emoji) {
        return std::make_unique<Button>(label, discord::components::ButtonStyle::SUCCESS, custom_id, emoji);
    }

    std::unique_ptr<discord::components::Button> discord::components::Button::danger(const std::string& label,
                                           const std::string& custom_id,
                                           const std::optional<std::string>& emoji) {
        return std::make_unique<Button>(label, discord::components::ButtonStyle::DANGER, custom_id, emoji);
    }

    std::unique_ptr<discord::components::Button> discord::components::Button::link(const std::string& label,
                                         const std::string& url,
                                         const std::optional<std::string>& emoji) {
        return std::make_unique<Button>(label, url, emoji);
    }

    std::unique_ptr<discord::components::Button> discord::components::Button::premium(const std::string& label,
                                           const std::string& sku_id,
                                           const std::optional<std::string>& emoji) {
        // Premium buttons would require SKU integration
        // For now, return a primary button with SKU info in custom_id
        return std::make_unique<Button>(label, discord::components::ButtonStyle::PREMIUM, sku_id, emoji);
    }

} // namespace discord::components