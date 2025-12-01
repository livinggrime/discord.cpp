#include "../include/discord/components/component_json.h"
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
#include <fstream>
#include <algorithm>

namespace discord::components {

    nlohmann::json ComponentJson::serialize_component(const IComponent& component) {
        return component.to_json();
    }

    nlohmann::json ComponentJson::serialize_components(const std::vector<std::shared_ptr<IComponent>>& components) {
        nlohmann::json json = nlohmann::json::array();
        for (const auto& component : components) {
            if (component) {
                json.push_back(component->to_json());
            }
        }
        return json;
    }

    std::unique_ptr<IComponent> ComponentJson::deserialize_component(const nlohmann::json& json) {
        if (!json.contains("type")) {
            throw std::invalid_argument("JSON missing required 'type' field");
        }

        ComponentType type = static_cast<ComponentType>(json["type"].get<int>());
        
        switch (type) {
            case ComponentType::BUTTON:
                return deserialize_button(json);
            case ComponentType::STRING_SELECT:
                return deserialize_string_select(json);
            case ComponentType::USER_SELECT:
                return deserialize_user_select(json);
            case ComponentType::ROLE_SELECT:
                return deserialize_role_select(json);
            case ComponentType::CHANNEL_SELECT:
                return deserialize_channel_select(json);
            case ComponentType::MENTIONABLE_SELECT:
                return deserialize_mentionable_select(json);
            case ComponentType::TEXT_INPUT:
                return deserialize_text_input(json);
            case ComponentType::ACTION_ROW:
                return deserialize_action_row(json);
            case ComponentType::SECTION:
                return deserialize_section(json);
            case ComponentType::CONTAINER:
                return deserialize_container(json);
            case ComponentType::TEXT_DISPLAY:
                return deserialize_text_display(json);
            case ComponentType::THUMBNAIL:
                return deserialize_thumbnail(json);
            case ComponentType::MEDIA_GALLERY:
                return deserialize_media_gallery(json);
            case ComponentType::FILE:
                return deserialize_file(json);
            case ComponentType::SEPARATOR:
                return deserialize_separator(json);
            case ComponentType::LABEL:
                return deserialize_label(json);
            default:
                throw std::invalid_argument("Unknown component type: " + std::to_string(static_cast<int>(type)));
        }
    }

    std::vector<std::shared_ptr<IComponent>> ComponentJson::deserialize_components(const nlohmann::json& json_array) {
        if (!json_array.is_array()) {
            throw std::invalid_argument("JSON must be an array for component deserialization");
        }

        std::vector<std::shared_ptr<IComponent>> components;
        components.reserve(json_array.size());
        
        for (const auto& component_json : json_array) {
            components.push_back(deserialize_component(component_json));
        }
        
        return components;
    }

    ComponentType ComponentJson::detect_component_type(const nlohmann::json& json) {
        if (!json.contains("type")) {
            return ComponentType::UNKNOWN;
        }
        
        try {
            return static_cast<ComponentType>(json["type"].get<int>());
        } catch (...) {
            return ComponentType::UNKNOWN;
        }
    }

    bool ComponentJson::is_valid_component_json(const nlohmann::json& json) {
        if (!json.contains("type")) {
            return false;
        }

        ComponentType type = detect_component_type(json);
        return type != ComponentType::UNKNOWN;
    }

    ValidationResult ComponentJson::validate_component_json(const nlohmann::json& json) {
        ValidationResult result;
        
        try {
            auto component = deserialize_component(json);
            if (component) {
                return component->validate();
            }
            result.valid = false;
            result.errors.push_back("Failed to deserialize component");
        } catch (const std::exception& e) {
            result.valid = false;
            result.errors.push_back(e.what());
        }
        
        return result;
    }

    std::string ComponentJson::get_serialization_error(const nlohmann::json& json) {
        // TODO: Implement detailed error reporting
        (void)json; // Suppress unused parameter warning
        return "Component serialization error";
    }

    std::string ComponentJson::get_deserialization_error(const nlohmann::json& json) {
        // TODO: Implement detailed error reporting
        (void)json; // Suppress unused parameter warning
        return "Component deserialization error";
    }

    std::string ComponentJson::to_pretty_string(const IComponent& component) {
        return component.to_json().dump(2);
    }

    std::string ComponentJson::to_pretty_string(const std::vector<std::shared_ptr<IComponent>>& components) {
        return serialize_components(components).dump(2);
    }

    std::string ComponentJson::to_pretty_string(const nlohmann::json& json) {
        return json.dump(2);
    }

    bool ComponentJson::save_component_to_file(const IComponent& component, const std::string& filepath) {
        try {
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            file << component.to_json().dump(2);
            return file.good();
        } catch (...) {
            return false;
        }
    }

    bool ComponentJson::save_components_to_file(const std::vector<std::shared_ptr<IComponent>>& components, const std::string& filepath) {
        try {
            std::ofstream file(filepath);
            if (!file.is_open()) {
                return false;
            }
            
            file << serialize_components(components).dump(2);
            return file.good();
        } catch (...) {
            return false;
        }
    }

    std::unique_ptr<IComponent> ComponentJson::load_component_from_file(const std::string& filepath) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                return nullptr;
            }
            
            nlohmann::json json;
            file >> json;
            return deserialize_component(json);
        } catch (...) {
            return nullptr;
        }
    }

    std::vector<std::shared_ptr<IComponent>> ComponentJson::load_components_from_file(const std::string& filepath) {
        try {
            std::ifstream file(filepath);
            if (!file.is_open()) {
                return {};
            }
            
            nlohmann::json json;
            file >> json;
            return deserialize_components(json);
        } catch (...) {
            return {};
        }
    }

    // Component-specific deserializers
    std::unique_ptr<Button> ComponentJson::deserialize_button(const nlohmann::json& json) {
        // TODO: Implement button deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<StringSelect> ComponentJson::deserialize_string_select(const nlohmann::json& json) {
        // TODO: Implement string select deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<UserSelect> ComponentJson::deserialize_user_select(const nlohmann::json& json) {
        // TODO: Implement user select deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<RoleSelect> ComponentJson::deserialize_role_select(const nlohmann::json& json) {
        // TODO: Implement role select deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<ChannelSelect> ComponentJson::deserialize_channel_select(const nlohmann::json& json) {
        // TODO: Implement channel select deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<MentionableSelect> ComponentJson::deserialize_mentionable_select(const nlohmann::json& json) {
        // TODO: Implement mentionable select deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<TextInput> ComponentJson::deserialize_text_input(const nlohmann::json& json) {
        // TODO: Implement text input deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<ActionRow> ComponentJson::deserialize_action_row(const nlohmann::json& json) {
        // TODO: Implement action row deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<Section> ComponentJson::deserialize_section(const nlohmann::json& json) {
        // TODO: Implement section deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<Container> ComponentJson::deserialize_container(const nlohmann::json& json) {
        // TODO: Implement container deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<TextDisplay> ComponentJson::deserialize_text_display(const nlohmann::json& json) {
        // TODO: Implement text display deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<Thumbnail> ComponentJson::deserialize_thumbnail(const nlohmann::json& json) {
        // TODO: Implement thumbnail deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<MediaGallery> ComponentJson::deserialize_media_gallery(const nlohmann::json& json) {
        // TODO: Implement media gallery deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<File> ComponentJson::deserialize_file(const nlohmann::json& json) {
        // TODO: Implement file deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<Separator> ComponentJson::deserialize_separator(const nlohmann::json& json) {
        // TODO: Implement separator deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

    std::unique_ptr<Label> ComponentJson::deserialize_label(const nlohmann::json& json) {
        // TODO: Implement label deserialization
        (void)json; // Suppress unused parameter warning
        return nullptr;
    }

} // namespace discord::components