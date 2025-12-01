/**
 * @file component_builder.h
 * @brief Fluent builder pattern for creating components
 * 
 * Provides a fluent interface for building complex component layouts.
 */

#pragma once

#include "component_base.h"
#include "button.h"
#include "select_menu.h"
#include "text_input.h"
#include "action_row.h"
#include "section.h"
#include "container.h"
#include "content_display.h"
#include <memory>
#include <vector>
#include <functional>

namespace discord::components {

    /**
     * @brief Builder class for creating components with fluent interface
     */
    class ComponentBuilder {
    private:
        std::vector<std::shared_ptr<IComponent>> components_;
        std::shared_ptr<IComponent> current_component_;

    public:
        ComponentBuilder() = default;
        ~ComponentBuilder() = default;

        // Button builders
        ComponentBuilder& button(const std::string& label,
                                 const std::string& custom_id = "",
                                 const std::string& style = "primary");

        ComponentBuilder& primary_button(const std::string& label,
                                         const std::string& custom_id = "");

        ComponentBuilder& secondary_button(const std::string& label,
                                           const std::string& custom_id = "");

        ComponentBuilder& success_button(const std::string& label,
                                        const std::string& custom_id = "");

        ComponentBuilder& danger_button(const std::string& label,
                                       const std::string& custom_id = "");

        ComponentBuilder& link_button(const std::string& label,
                                      const std::string& url);

        ComponentBuilder& button_emoji(const std::string& emoji);
        ComponentBuilder& button_disabled(bool disabled = true);

        // Select menu builders
        ComponentBuilder& string_select(const std::string& custom_id,
                                       const std::vector<SelectOption>& options = {},
                                       const std::string& placeholder = "",
                                       int min_values = 1,
                                       int max_values = 25);

        ComponentBuilder& user_select(const std::string& custom_id,
                                       const std::string& placeholder = "",
                                       int min_values = 1,
                                       int max_values = 25);

        ComponentBuilder& role_select(const std::string& custom_id,
                                       const std::string& placeholder = "",
                                       int min_values = 1,
                                       int max_values = 25);

        ComponentBuilder& channel_select(const std::string& custom_id,
                                         const std::vector<std::string>& channel_types = {},
                                         const std::string& placeholder = "",
                                         int min_values = 1,
                                         int max_values = 25);

        ComponentBuilder& mentionable_select(const std::string& custom_id,
                                            const std::string& placeholder = "",
                                            int min_values = 1,
                                            int max_values = 25);

        ComponentBuilder& select_placeholder(const std::string& placeholder);
        ComponentBuilder& select_min_values(int min_values);
        ComponentBuilder& select_max_values(int max_values);
        ComponentBuilder& select_option(const std::string& label,
                                       const std::string& value,
                                       const std::string& description = "",
                                       const std::string& emoji = "",
                                       bool default_ = false);

        // Text input builders
        ComponentBuilder& text_input(const std::string& custom_id,
                                     const std::string& label,
                                     const std::string& style = "short",
                                     const std::string& value = "",
                                     const std::string& placeholder = "",
                                     bool required = false,
                                     int min_length = 0,
                                     int max_length = 4000);

        ComponentBuilder& short_text(const std::string& custom_id,
                                     const std::string& label,
                                     const std::string& value = "",
                                     const std::string& placeholder = "",
                                     bool required = false,
                                     int min_length = 0,
                                     int max_length = 4000);

        ComponentBuilder& paragraph_text(const std::string& custom_id,
                                        const std::string& label,
                                        const std::string& value = "",
                                        const std::string& placeholder = "",
                                        bool required = false,
                                        int min_length = 0,
                                        int max_length = 4000);

        ComponentBuilder& text_input_style(const std::string& style);
        ComponentBuilder& text_input_value(const std::string& value);
        ComponentBuilder& text_input_placeholder(const std::string& placeholder);
        ComponentBuilder& text_input_required(bool required = true);
        ComponentBuilder& text_input_min_length(int min_length);
        ComponentBuilder& text_input_max_length(int max_length);

        // Content display builders
        ComponentBuilder& text(const std::string& content);
        ComponentBuilder& thumbnail(const std::string& url,
                                    const std::string& alt_text = "",
                                    int width = 0,
                                    int height = 0);

        ComponentBuilder& media_gallery(const std::vector<std::string>& items,
                                        const std::string& alt_text = "");

        ComponentBuilder& file(const std::string& url,
                               const std::string& filename,
                               int size_bytes = 0);

        ComponentBuilder& separator(bool decorative = true, int spacing = 0);
        ComponentBuilder& label(const std::string& text,
                                 const std::string& for_component = "");

        // Container builders
        ComponentBuilder& action_row();
        ComponentBuilder& section(const std::string& text);
        ComponentBuilder& container();

        ComponentBuilder& add_to_current(std::shared_ptr<IComponent> component);
        ComponentBuilder& finish_current();

        // Layout operations
        ComponentBuilder& new_row();
        ComponentBuilder& new_section(const std::string& text = "");
        ComponentBuilder& new_container();

        // Building
        std::shared_ptr<IComponent> build();
        std::vector<std::shared_ptr<IComponent>> build_all();
        nlohmann::json build_json();

        // Utility methods
        ComponentBuilder& clear();
        ComponentBuilder& reset_current();

        // Static factory methods
        static ComponentBuilder create();
        static ComponentBuilder from_component(std::shared_ptr<IComponent> component);
        static ComponentBuilder from_components(const std::vector<std::shared_ptr<IComponent>>& components);

        // Callback-based building
        ComponentBuilder& with_button(const std::function<ComponentBuilder&(ComponentBuilder&)>& builder);
        ComponentBuilder& with_select(const std::function<ComponentBuilder&(ComponentBuilder&)>& builder);
        ComponentBuilder& with_text_input(const std::function<ComponentBuilder&(ComponentBuilder&)>& builder);
        ComponentBuilder& with_row(const std::function<ComponentBuilder&(ComponentBuilder&)>& builder);
        ComponentBuilder& with_section(const std::function<ComponentBuilder&(ComponentBuilder&)>& builder);
    };

    // Convenience functions
    ComponentBuilder buttons();
    ComponentBuilder selects();
    ComponentBuilder text_inputs();
    ComponentBuilder content();
    ComponentBuilder layout();

} // namespace discord::components