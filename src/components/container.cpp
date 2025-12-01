#include "../include/discord/components/container.h"
#include "../include/discord/components/section.h"
#include "../include/discord/components/content_display.h"
#include "../include/discord/utils/types.h"
#include <stdexcept>

namespace discord::components {

    Container::Container(const std::vector<std::shared_ptr<IComponent>>& components)
        : ContainerComponent(components) {
        if (components_.size() > 10) {
            throw std::invalid_argument("Container cannot contain more than 10 components");
        }
    }

    nlohmann::json Container::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::CONTAINER);
        
        nlohmann::json components_array = nlohmann::json::array();
        for (const auto& component : components_) {
            if (component) {
                components_array.push_back(component->to_json());
            }
        }
        json["components"] = components_array;
        
        return json;
    }

    bool Container::validate() const {
        if (components_.size() > 10) {
            return false;
        }

        // Validate each component
        for (const auto& component : components_) {
            if (!component || !component->validate()) {
                return false;
            }
        }

        return true;
    }

    std::unique_ptr<IComponent> Container::clone() const {
        std::vector<std::shared_ptr<IComponent>> cloned_components;
        cloned_components.reserve(components_.size());
        
        for (const auto& component : components_) {
            if (component) {
                cloned_components.push_back(std::shared_ptr<IComponent>(component->clone().release()));
            }
        }
        
        return std::make_unique<Container>(cloned_components);
    }

    bool Container::can_add_component(std::shared_ptr<IComponent> component) const {
        if (!component || components_.size() >= 10) {
            return false;
        }
        return true;
    }

    std::unique_ptr<Container> Container::create(const std::vector<std::shared_ptr<IComponent>>& components) {
        return std::make_unique<Container>(components);
    }

    std::unique_ptr<Container> Container::with_sections(const std::vector<std::shared_ptr<Section>>& sections) {
        std::vector<std::shared_ptr<IComponent>> components;
        components.reserve(sections.size());
        
        for (const auto& section : sections) {
            if (section) {
                components.push_back(std::static_pointer_cast<IComponent>(section));
            }
        }
        
        return std::make_unique<Container>(components);
    }

    std::unique_ptr<Container> Container::with_action_rows(const std::vector<std::shared_ptr<ActionRow>>& action_rows) {
        std::vector<std::shared_ptr<IComponent>> components;
        components.reserve(action_rows.size());
        
        for (const auto& action_row : action_rows) {
            if (action_row) {
                components.push_back(std::static_pointer_cast<IComponent>(action_row));
            }
        }
        
        return std::make_unique<Container>(components);
    }

    std::unique_ptr<Container> Container::mixed(const std::vector<std::shared_ptr<IComponent>>& components) {
        return std::make_unique<Container>(components);
    }

} // namespace discord::components