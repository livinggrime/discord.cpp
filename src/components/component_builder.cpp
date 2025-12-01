#include "../include/discord/components/component_builder.h"
#include "../include/discord/components/component_base.h"
#include "../include/discord/components/button.h"
#include "../include/discord/components/select_menu.h"
#include "../include/discord/components/text_input.h"
#include "../include/discord/components/action_row.h"
#include "../include/discord/components/section.h"
#include "../include/discord/components/container.h"
#include "../include/discord/components/content_display.h"
#include "../include/discord/utils/types.h"
#include <stdexcept>
#include <algorithm>

namespace discord::components {

    ComponentBuilder::ComponentBuilder() : current_component_(nullptr) {}

    ComponentBuilder::~ComponentBuilder() = default;

    // Button builders
    ComponentBuilder& ComponentBuilder::button(const std::string& label,
                                             const std::string& custom_id,
                                             const std::string& style,
                                             const std::optional<std::string>& emoji,
                                             bool disabled) {
        auto button = std::make_shared<Button>(label, style, custom_id, emoji, disabled);
        add_component(button);
        return *this;
    }

    ComponentBuilder& ComponentBuilder::primary_button(const std::string& label,
                                                const std::string& custom_id,
                                                const std::optional<std::string>& emoji,
                                                bool disabled) {
        return button(label, "primary", custom_id, emoji, disabled);
    }

    ComponentBuilder& ComponentBuilder::secondary_button(const std::string& label,
                                                   const std::string& custom_id,
                                                   const std::optional<std::string>& emoji,
                                                   bool disabled) {
        return button(label, "secondary", custom_id, emoji, disabled);
    }

    ComponentBuilder& ComponentBuilder::success_button(const std::string& label,
                                                const std::string& custom_id,
                                                const std::optional<std::string>& emoji,
                                                bool disabled) {
        return button(label, "success", custom_id, emoji, disabled);
    }

    ComponentBuilder& ComponentBuilder::danger_button(const std::string& label,
                                               const std::string& custom_id,
                                               const std::optional<std::string>& emoji,
                                               bool disabled) {
        return button(label, "danger", custom_id, emoji, disabled);
    }

    ComponentBuilder& ComponentBuilder::link_button(const std::string& label,
                                          const std::string& url,
                                          const std::optional<std::string>& emoji,
                                          bool disabled) {
        return button(label, url, emoji, disabled);
    }

    ComponentBuilder& ComponentBuilder::premium_button(const std::string& label,
                                                const std::string& sku_id,
                                                const std::optional<std::string>& emoji,
                                                bool disabled) {
        return button(label, "premium", sku_id, emoji, disabled);
    }

    ComponentBuilder& ComponentBuilder::button_emoji(const std::string& emoji) {
        if (current_component_ && current_component_->get_type() == ComponentType::BUTTON) {
            auto button = std::static_pointer_cast<Button>(current_component_);
            if (button) {
                button->set_emoji(emoji);
            }
        }
        return *this;
    }

    // Select menu builders
    ComponentBuilder& ComponentBuilder::string_select(const std::string& custom_id,
                                                   const std::vector<SelectOption>& options,
                                                   const std::string& placeholder,
                                                   int min_values,
                                                   int max_values,
                                                   bool disabled) {
        auto select = std::make_shared<StringSelect>(custom_id, options, placeholder, min_values, max_values, disabled);
        add_component(select);
        return *this;
    }

    ComponentBuilder& ComponentBuilder::user_select(const std::string& custom_id,
                                                const std::string& placeholder,
                                                int min_values,
                                                int max_values,
                                                bool disabled) {
        auto select = std::make_shared<UserSelect>(custom_id, placeholder, min_values, max_values, disabled);
        add_component(select);
        return *this;
    }

    ComponentBuilder& ComponentBuilder::role_select(const std::string& custom_id,
                                                const std::string& placeholder,
                                                int min_values,
                                                int max_values,
                                                bool disabled) {
        auto select = std::make_shared<RoleSelect>(custom_id, placeholder, min_values, max_values, disabled);
        add_component(select);
        return *this;
    }

    ComponentBuilder& ComponentBuilder::channel_select(const std::string& custom_id,
                                                   const std::vector<std::string>& channel_types,
                                                   const std::string& placeholder,
                                                   int min_values,
                                                   int max_values,
                                                   bool disabled) {
        auto select = std::make_shared<ChannelSelect>(custom_id, channel_types, placeholder, min_values, max_values, disabled);
        add_component(select);
        return *this;
    }

    ComponentBuilder& ComponentBuilder::mentionable_select(const std::string& custom_id,
                                                        const std::string& placeholder,
                                                        int min_values,
                                                        int max_values,
                                                        bool disabled) {
        auto select = std::make_shared<MentionableSelect>(custom_id, placeholder, min_values, max_values, disabled);
        add_component(select);
        return *this;
    }

    ComponentBuilder& ComponentBuilder::select_placeholder(const std::string& placeholder) {
        if (current_component_ && current_component_->get_type() == ComponentType::STRING_SELECT) {
            auto select = std::static_pointer_cast<StringSelect>(current_component_);
            if (select) {
                select->set_placeholder(placeholder);
            }
        }
        return *this;
    }

    ComponentBuilder& ComponentBuilder::select_min_values(int min_values) {
        if (current_component_ && current_component_->get_type() == ComponentType::STRING_SELECT) {
            auto select = std::static_pointer_cast<StringSelect>(current_component_);
            if (select) {
                select->set_min_values(min_values);
            }
        }
        return *this;
    }

    ComponentBuilder& ComponentBuilder::select_max_values(int max_values) {
        if (current_component_ && current_component_->get_type() == ComponentType::STRING_SELECT) {
            auto select = std::static_pointer_cast<StringSelect>(current_component_);
            if (select) {
                select->set_max_values(max_values);
            }
        }
        return *this;
    }

    ComponentBuilder& ComponentBuilder::select_option(const std::string& label,
                                             const std::string& value,
                                             const std::optional<std::string>& description,
                                             const std::optional<std::string>& emoji,
                                             bool default_) {
        if (current_component_ && current_component_->get_type() == ComponentType::STRING_SELECT) {
            auto select = std::static_pointer_cast<StringSelect>(current_component_);
            if (select) {
                select->add_option(SelectOption(label, value, description, emoji, default_));
            }
        }
        return *this;
    }

    // Text input builders
    ComponentBuilder& ComponentBuilder::short_text(const std::string& custom_id,
                                                const std::string& label,
                                                const std::optional<std::string>& value,
                                                const std::optional<std::string>& placeholder,
                                                bool required,
                                                int min_length,
                                                int max_length) {
        auto text_input = std::make_shared<TextInput>(custom_id, label, TextInputStyle::SHORT, value, placeholder, required, min_length, max_length);
        add_component(text_input);
        return *this;
    }

    ComponentBuilder& ComponentBuilder::paragraph(const std::string& custom_id,
                                                 const std::string& label,
                                                 const std::optional<std::string>& value,
                                                 const std::optional<std::string>& placeholder,
                                                 bool required,
                                                 int min_length,
                                                 int max_length) {
        auto text_input = std::make_shared<TextInput>(custom_id, label, TextInputStyle::PARAGRAPH, value, placeholder, required, min_length, max_length);
        add_component(text_input);
        return *this;
    }

    ComponentBuilder& ComponentBuilder::text_input_style(TextInputStyle style) {
        if (current_component_ && current_component_->get_type() == ComponentType::TEXT_INPUT) {
            auto text_input = std::static_pointer_cast<TextInput>(current_component_);
            if (text_input) {
                text_input->set_style(style);
            }
        }
        return *this;
    }

    ComponentBuilder& ComponentBuilder::text_input_value(const std::string& value) {
        if (current_component_ && current_component_->get_type() == ComponentType::TEXT_INPUT) {
            auto text_input = std::static_pointer_cast<TextInput>(current_component_);
            if (text_input) {
                text_input->set_value(value);
            }
        }
        return *this;
    }

    ComponentBuilder& ComponentBuilder::text_input_placeholder(const std::string& placeholder) {
        if (current_component_ && current_component_->get_type() == ComponentType::TEXT_INPUT) {
            auto text_input = std::static_pointer_cast<TextInput>(current_component_);
            if (text_input) {
                text_input->set_placeholder(placeholder);
            }
        }
        return *this;
    }

    ComponentBuilder& ComponentBuilder::text_input_required(bool required) {
        if (current_component_ && current_component_->get_type() == ComponentType::TEXT_INPUT) {
            auto text_input = std::static_pointer_cast<TextInput>(current_component_);
            if (text_input) {
                text_input->set_required(required);
            }
        }
        return *this;
    }

    ComponentBuilder& ComponentBuilder::text_input_min_length(int min_length) {
        if (current_component_ && current_component_->get_type() == ComponentType::TEXT_INPUT) {
            auto text_input = std::static_pointer_cast<TextInput>(current_component_);
            if (text_input) {
                text_input->set_min_length(min_length);
            }
        }
        return *this;
    }

    ComponentBuilder& ComponentBuilder::text_input_max_length(int max_length) {
        if (current_component_ && current_component_->get_type() == ComponentType::TEXT_INPUT) {
            auto text_input = std::static_pointer_cast<TextInput>(current_component_);
            if (text_input) {
                text_input->set_max_length(max_length);
            }
        }
        return *this;
    }

    // Layout builders
    ComponentBuilder& ComponentBuilder::new_row() {
        current_component_ = std::make_unique<ActionRow>();
        return *this;
    }

    ComponentBuilder& ComponentBuilder::new_section(const std::string& text) {
        current_component_ = std::make_unique<Section>(text);
        return *this;
    }

    ComponentBuilder& ComponentBuilder::new_container() {
        current_component_ = std::make_unique<Container>();
        return *this;
    }

    ComponentBuilder& ComponentBuilder::finish_current() {
        if (current_component_) {
            components_.push_back(current_component_);
            current_component_ = nullptr;
        }
        return *this;
    }

    ComponentBuilder& ComponentBuilder::finish_current() {
        return finish_current();
    }

    ComponentBuilder& ComponentBuilder::build() {
        if (!current_component_) {
            throw std::runtime_error("No component to build");
        }
        
        auto result = current_component_;
        current_component_ = nullptr;
        return result;
    }

    ComponentBuilder& ComponentBuilder::build_all() {
        if (components_.empty()) {
            throw std::runtime_error("No components to build");
        }
        
        auto result = components_;
        components_.clear();
        return result;
    }

    // Convenience methods
    ComponentBuilder& ComponentBuilder::with_button(const std::function<ComponentBuilder&>& builder) {
        return builder.button("", "", "", "", "", "", "");
    }

    ComponentBuilder& ComponentBuilder::with_select(const std::function<ComponentBuilder&>& builder) {
        return builder.string_select("", {}, {}, "", 1, 25);
    }

    ComponentBuilder& ComponentBuilder::with_text_input(const std::function<ComponentBuilder&>& builder) {
        return builder.short_text("", "", "", "", false, false, 0, 4000);
    }

    ComponentBuilder& ComponentBuilder::with_row(const std::function<ComponentBuilder&>& builder) {
        return builder.new_row()
            .primary_button("", "")
            .secondary_button("", "")
            .finish_current();
    }

    ComponentBuilder& ComponentBuilder::with_section(const std::function<ComponentBuilder&>& builder) {
        return builder.new_section("Text content")
            .with_button("", "Click me", "click_me")
            .finish_current();
    }

    ComponentBuilder& ComponentBuilder::with_container(const std::function<ComponentBuilder&>& builder) {
        return builder.new_container();
    }

} // namespace discord::components