#include "../include/discord/components/action_row.h"
#include "../include/discord/components/button.h"
#include "../include/discord/components/select_menu.h"
#include "../include/discord/components/text_input.h"
#include "../include/discord/utils/types.h"
#include <algorithm>
#include <stdexcept>

namespace discord::components {

    ActionRow::ActionRow(const std::vector<std::shared_ptr<IComponent>>& components)
        : ContainerComponent(components) {
        validate_component_constraints();
    }

    void ActionRow::validate_component_constraints() {
        if (components_.size() > 5) {
            throw std::invalid_argument("ActionRow cannot contain more than 5 components");
        }

        // Check if this is a button-only row
        if (is_button_only()) {
            for (const auto& component : components_) {
                if (component->get_type() != ComponentType::BUTTON) {
                    throw std::invalid_argument("ActionRow with buttons can only contain button components");
                }
            }
        }
    }

    nlohmann::json ActionRow::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::ACTION_ROW);
        
        nlohmann::json components_array = nlohmann::json::array();
        for (const auto& component : components_) {
            if (component) {
                components_array.push_back(component->to_json());
            }
        }
        json["components"] = components_array;
        
        return json;
    }

    bool ActionRow::validate() const {
        if (components_.size() > 5) {
            return false;
        }

        // Validate each component
        for (const auto& component : components_) {
            if (!component || !component->validate()) {
                return false;
            }
        }

        // Check button-only constraint
        if (is_button_only()) {
            return std::all_of(components_.begin(), components_.end(),
                [](const std::shared_ptr<IComponent>& comp) {
                    return comp && comp->get_type() == ComponentType::BUTTON;
                });
        }

        return true;
    }

    std::unique_ptr<IComponent> ActionRow::clone() const {
        std::vector<std::shared_ptr<IComponent>> cloned_components;
        cloned_components.reserve(components_.size());
        
        for (const auto& component : components_) {
            if (component) {
                cloned_components.push_back(std::shared_ptr<IComponent>(component->clone().release()));
            }
        }
        
        return std::make_unique<ActionRow>(cloned_components);
    }

    bool ActionRow::can_add_component(std::shared_ptr<IComponent> component) const {
        if (!component || components_.size() >= 5) {
            return false;
        }

        // If this is a button-only row, only allow buttons
        if (is_button_only()) {
            return component->get_type() == ComponentType::BUTTON;
        }

        // If row already has buttons, only allow buttons
        bool has_buttons = std::any_of(components_.begin(), components_.end(),
            [](const std::shared_ptr<IComponent>& comp) {
                return comp && comp->get_type() == ComponentType::BUTTON;
            });

        if (has_buttons) {
            return component->get_type() == ComponentType::BUTTON;
        }

        return true;
    }

    bool ActionRow::is_button_only() const {
        return std::any_of(components_.begin(), components_.end(),
            [](const std::shared_ptr<IComponent>& comp) {
                return comp && comp->get_type() == ComponentType::BUTTON;
            });
    }

    std::unique_ptr<ActionRow> ActionRow::create(const std::vector<std::shared_ptr<IComponent>>& components) {
        return std::make_unique<ActionRow>(components);
    }

    std::unique_ptr<ActionRow> ActionRow::with_buttons(const std::vector<std::shared_ptr<Button>>& buttons) {
        std::vector<std::shared_ptr<IComponent>> components;
        components.reserve(buttons.size());
        
        for (const auto& button : buttons) {
            if (button) {
                components.push_back(std::static_pointer_cast<IComponent>(button));
            }
        }
        
        return std::make_unique<ActionRow>(components);
    }

    std::unique_ptr<ActionRow> ActionRow::with_selects(const std::vector<std::shared_ptr<SelectMenu>>& selects) {
        std::vector<std::shared_ptr<IComponent>> components;
        components.reserve(selects.size());
        
        for (const auto& select : selects) {
            if (select) {
                components.push_back(std::static_pointer_cast<IComponent>(select));
            }
        }
        
        return std::make_unique<ActionRow>(components);
    }

    std::unique_ptr<ActionRow> ActionRow::with_text_inputs(const std::vector<std::shared_ptr<TextInput>>& text_inputs) {
        std::vector<std::shared_ptr<IComponent>> components;
        components.reserve(text_inputs.size());
        
        for (const auto& text_input : text_inputs) {
            if (text_input) {
                components.push_back(std::static_pointer_cast<IComponent>(text_input));
            }
        }
        
        return std::make_unique<ActionRow>(components);
    }

} // namespace discord::components