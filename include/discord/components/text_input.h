/**
 * @file text_input.h
 * @brief Text input component for modals
 * 
 * Provides text input components used in modal dialogs.
 */

#pragma once

#include "component_base.h"
#include <optional>

namespace discord::components {

    /**
     * @brief Text input component for modals
     */
    class TextInput : public InteractiveComponent {
    private:
        TextInputStyle style_;
        std::string label_;
        std::optional<std::string> value_;
        std::optional<std::string> placeholder_;
        bool required_ = false;
        int min_length_ = 0;
        int max_length_ = 4000;

    public:
        TextInput(const std::string& custom_id,
                 const std::string& label,
                 TextInputStyle style = TextInputStyle::SHORT,
                 const std::optional<std::string>& value = std::nullopt,
                 const std::optional<std::string>& placeholder = std::nullopt,
                 bool required = false,
                 int min_length = 0,
                 int max_length = 4000);

        ~TextInput() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::TEXT_INPUT; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // TextInput specific methods
        void set_style(TextInputStyle style) { style_ = style; }
        TextInputStyle get_style() const { return style_; }

        void set_label(const std::string& label) { label_ = label; }
        const std::string& get_label() const { return label_; }

        void set_value(const std::optional<std::string>& value) { value_ = value; }
        const std::optional<std::string>& get_value() const { return value_; }

        void set_placeholder(const std::optional<std::string>& placeholder) { placeholder_ = placeholder; }
        const std::optional<std::string>& get_placeholder() const { return placeholder_; }

        void set_required(bool required) { required_ = required; }
        bool is_required() const { return required_; }

        void set_min_length(int min_length) { min_length_ = min_length; }
        int get_min_length() const { return min_length_; }

        void set_max_length(int max_length) { max_length_ = max_length; }
        int get_max_length() const { return max_length_; }

        // Static factory methods
        static std::unique_ptr<TextInput> short_text(const std::string& custom_id,
                                                    const std::string& label,
                                                    const std::optional<std::string>& value = std::nullopt,
                                                    const std::optional<std::string>& placeholder = std::nullopt,
                                                    bool required = false,
                                                    int min_length = 0,
                                                    int max_length = 4000);

        static std::unique_ptr<TextInput> paragraph(const std::string& custom_id,
                                                   const std::string& label,
                                                   const std::optional<std::string>& value = std::nullopt,
                                                   const std::optional<std::string>& placeholder = std::nullopt,
                                                   bool required = false,
                                                   int min_length = 0,
                                                   int max_length = 4000);

        static std::unique_ptr<TextInput> create(const std::string& custom_id,
                                                const std::string& label,
                                                TextInputStyle style = TextInputStyle::SHORT,
                                                const std::optional<std::string>& value = std::nullopt,
                                                const std::optional<std::string>& placeholder = std::nullopt,
                                                bool required = false,
                                                int min_length = 0,
                                                int max_length = 4000);
    };

} // namespace discord::components