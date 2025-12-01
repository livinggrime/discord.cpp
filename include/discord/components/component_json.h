#pragma once

#include <memory>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>
#include "../utils/types.h"

namespace discord::components {

    // Forward declarations
    class IComponent;
    class Button;
    class StringSelect;
    class UserSelect;
    class RoleSelect;
    class ChannelSelect;
    class MentionableSelect;
    class TextInput;
    class ActionRow;
    class Section;
    class Container;
    class TextDisplay;
    class Thumbnail;
    class MediaGallery;
    class File;
    class Separator;
    class Label;

    /**
     * @brief JSON serialization and deserialization utilities for Discord components
     */
    class ComponentJson {
    public:
        // Serialization methods
        static nlohmann::json serialize_component(const IComponent& component);
        static nlohmann::json serialize_components(const std::vector<std::shared_ptr<IComponent>>& components);

        // Deserialization methods
        static std::unique_ptr<IComponent> deserialize_component(const nlohmann::json& json);
        static std::vector<std::shared_ptr<IComponent>> deserialize_components(const nlohmann::json& json_array);

        // Type detection and validation
        static ComponentType detect_component_type(const nlohmann::json& json);
        static bool is_valid_component_json(const nlohmann::json& json);
        static ValidationResult validate_component_json(const nlohmann::json& json);

        // Error handling
        static std::string get_serialization_error(const nlohmann::json& json);
        static std::string get_deserialization_error(const nlohmann::json& json);

        // Pretty printing
        static std::string to_pretty_string(const IComponent& component);
        static std::string to_pretty_string(const std::vector<std::shared_ptr<IComponent>>& components);
        static std::string to_pretty_string(const nlohmann::json& json);

        // File I/O
        static bool save_component_to_file(const IComponent& component, const std::string& filepath);
        static bool save_components_to_file(const std::vector<std::shared_ptr<IComponent>>& components, const std::string& filepath);
        static std::unique_ptr<IComponent> load_component_from_file(const std::string& filepath);
        static std::vector<std::shared_ptr<IComponent>> load_components_from_file(const std::string& filepath);

    private:
        // Component-specific deserializers
        static std::unique_ptr<Button> deserialize_button(const nlohmann::json& json);
        static std::unique_ptr<StringSelect> deserialize_string_select(const nlohmann::json& json);
        static std::unique_ptr<UserSelect> deserialize_user_select(const nlohmann::json& json);
        static std::unique_ptr<RoleSelect> deserialize_role_select(const nlohmann::json& json);
        static std::unique_ptr<ChannelSelect> deserialize_channel_select(const nlohmann::json& json);
        static std::unique_ptr<MentionableSelect> deserialize_mentionable_select(const nlohmann::json& json);
        static std::unique_ptr<TextInput> deserialize_text_input(const nlohmann::json& json);
        static std::unique_ptr<ActionRow> deserialize_action_row(const nlohmann::json& json);
        static std::unique_ptr<Section> deserialize_section(const nlohmann::json& json);
        static std::unique_ptr<Container> deserialize_container(const nlohmann::json& json);
        static std::unique_ptr<TextDisplay> deserialize_text_display(const nlohmann::json& json);
        static std::unique_ptr<Thumbnail> deserialize_thumbnail(const nlohmann::json& json);
        static std::unique_ptr<MediaGallery> deserialize_media_gallery(const nlohmann::json& json);
        static std::unique_ptr<File> deserialize_file(const nlohmann::json& json);
        static std::unique_ptr<Separator> deserialize_separator(const nlohmann::json& json);
        static std::unique_ptr<Label> deserialize_label(const nlohmann::json& json);
    };

} // namespace discord::components