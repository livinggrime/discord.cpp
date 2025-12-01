/**
 * @file button.h
 * @brief Button component implementation
 * 
 * Provides button components with various styles and behaviors.
 */

#pragma once

#include "component_base.h"
#include <optional>

namespace discord::components {

    /**
     * @brief Button component class
     */
    class Button : public InteractiveComponent {
    private:
        ButtonStyle style_;
        std::string label_;
        std::optional<std::string> emoji_;
        std::optional<std::string> url_;
        bool disabled_ = false;

    public:
        /**
         * @brief Constructor for interactive button
         */
        Button(const std::string& label,
               ButtonStyle style = ButtonStyle::PRIMARY,
               const std::string& custom_id = "",
               const std::optional<std::string>& emoji = std::nullopt,
               bool disabled = false);

        /**
         * @brief Constructor for link button
         */
        Button(const std::string& label,
               const std::string& url,
               const std::optional<std::string>& emoji = std::nullopt,
               bool disabled = false);

        ~Button() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::BUTTON; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // Button-specific methods
        void set_style(ButtonStyle style) { style_ = style; }
        ButtonStyle get_style() const { return style_; }

        void set_label(const std::string& label) { label_ = label; }
        const std::string& get_label() const { return label_; }

        void set_emoji(const std::optional<std::string>& emoji) { emoji_ = emoji; }
        const std::optional<std::string>& get_emoji() const { return emoji_; }

        void set_url(const std::optional<std::string>& url) { url_ = url; }
        const std::optional<std::string>& get_url() const { return url_; }

        bool is_link_button() const { return url_.has_value(); }

        // Static factory methods
        static std::unique_ptr<Button> primary(const std::string& label, 
                                              const std::string& custom_id = "",
                                              const std::optional<std::string>& emoji = std::nullopt);

        static std::unique_ptr<Button> secondary(const std::string& label,
                                                const std::string& custom_id = "",
                                                const std::optional<std::string>& emoji = std::nullopt);

        static std::unique_ptr<Button> success(const std::string& label,
                                              const std::string& custom_id = "",
                                              const std::optional<std::string>& emoji = std::nullopt);

        static std::unique_ptr<Button> danger(const std::string& label,
                                             const std::string& custom_id = "",
                                             const std::optional<std::string>& emoji = std::nullopt);

        static std::unique_ptr<Button> link(const std::string& label,
                                           const std::string& url,
                                           const std::optional<std::string>& emoji = std::nullopt);

        static std::unique_ptr<Button> premium(const std::string& label,
                                              const std::string& sku_id,
                                              const std::optional<std::string>& emoji = std::nullopt);
    };

} // namespace discord::components