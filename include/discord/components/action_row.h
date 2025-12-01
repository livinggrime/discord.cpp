/**
 * @file action_row.h
 * @brief Action row container component
 * 
 * Provides action row containers for organizing interactive components.
 */

#pragma once

#include "component_base.h"
#include "button.h"
#include "select_menu.h"
#include "text_input.h"

namespace discord::components {

    /**
     * @brief Action row container component
     * 
     * Action rows are containers that can hold up to 5 components.
     * For buttons, action rows can only contain buttons.
     * For other components, action rows can contain any mix of components.
     */
    class ActionRow : public ContainerComponent {
    public:
        /**
         * @brief Constructor
         */
        ActionRow(const std::vector<std::shared_ptr<IComponent>>& components = {});

        ~ActionRow() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::ACTION_ROW; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // ActionRow specific methods
        bool can_add_component(std::shared_ptr<IComponent> component) const;
        bool is_button_only() const;
        bool is_full() const { return components_.size() >= 5; }

        // Static factory method
        static std::unique_ptr<ActionRow> create(const std::vector<std::shared_ptr<IComponent>>& components = {});

        // Convenience factory methods
        static std::unique_ptr<ActionRow> with_buttons(const std::vector<std::shared_ptr<Button>>& buttons);
        static std::unique_ptr<ActionRow> with_selects(const std::vector<std::shared_ptr<SelectMenu>>& selects);
        static std::unique_ptr<ActionRow> with_text_inputs(const std::vector<std::shared_ptr<TextInput>>& text_inputs);
    };

} // namespace discord::components