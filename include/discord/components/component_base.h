/**
 * @file component_base.h
 * @brief Base interfaces and abstract classes for Discord components
 * 
 * This file defines the fundamental interfaces that all components must implement,
 * providing a consistent API across all component types.
 */

#pragma once

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <variant>
#include <functional>
#include <nlohmann/json.hpp>

namespace discord::components {

    /**
     * @brief Component type enumeration
     */
    enum class ComponentType {
        ACTION_ROW = 1,
        BUTTON = 2,
        STRING_SELECT = 3,
        TEXT_INPUT = 4,
        USER_SELECT = 5,
        ROLE_SELECT = 6,
        MENTIONABLE_SELECT = 7,
        CHANNEL_SELECT = 8,
        SECTION = 9,
        CONTAINER = 10,
        TEXT_DISPLAY = 11,
        THUMBNAIL = 12,
        MEDIA_GALLERY = 13,
        FILE = 14,
        SEPARATOR = 15,
        LABEL = 16
    };

    /**
     * @brief Button style enumeration
     */
    enum class ButtonStyle {
        PRIMARY = 1,      // Blue
        SECONDARY = 2,    // Gray
        SUCCESS = 3,      // Green
        DANGER = 4,       // Red
        LINK = 5,         // Gray, navigates to URL
        PREMIUM = 6       // Requires premium subscription
    };

    /**
     * @brief Text input style enumeration
     */
    enum class TextInputStyle {
        SHORT = 1,        // Single line
        PARAGRAPH = 2     // Multi-line
    };

    /**
     * @brief Base interface for all components
     */
    class IComponent {
    public:
        virtual ~IComponent() = default;
        
        /**
         * @brief Get the component type
         */
        virtual ComponentType get_type() const = 0;
        
        /**
         * @brief Convert component to JSON
         */
        virtual nlohmann::json to_json() const = 0;
        
        /**
         * @brief Validate component configuration
         */
        virtual bool validate() const = 0;
        
        /**
         * @brief Get component's custom ID (if applicable)
         */
        virtual std::string get_custom_id() const { return ""; }
        
        /**
         * @brief Check if component is disabled
         */
        virtual bool is_disabled() const { return false; }
        
        /**
         * @brief Clone component
         */
        virtual std::unique_ptr<IComponent> clone() const = 0;
    };

    /**
     * @brief Base class for interactive components (buttons, select menus, text inputs)
     */
    class InteractiveComponent : public IComponent {
    protected:
        std::string custom_id_;
        bool disabled_ = false;
        
    public:
        InteractiveComponent(const std::string& custom_id, bool disabled = false)
            : custom_id_(custom_id), disabled_(disabled) {}
        
        virtual ~InteractiveComponent() = default;
        
        std::string get_custom_id() const override { return custom_id_; }
        bool is_disabled() const override { return disabled_; }
        
        void set_custom_id(const std::string& custom_id) { custom_id_ = custom_id; }
        void set_disabled(bool disabled) { disabled_ = disabled; }
    };

    /**
     * @brief Base class for container components (action rows, sections, containers)
     */
    class ContainerComponent : public IComponent {
    protected:
        std::vector<std::shared_ptr<IComponent>> components_;
        
    public:
        ContainerComponent(const std::vector<std::shared_ptr<IComponent>>& components = {})
            : components_(components) {}
        
        virtual ~ContainerComponent() = default;
        
        void add_component(std::shared_ptr<IComponent> component) {
            components_.push_back(component);
        }
        
        void remove_component(size_t index) {
            if (index < components_.size()) {
                components_.erase(components_.begin() + index);
            }
        }
        
        const std::vector<std::shared_ptr<IComponent>>& get_components() const {
            return components_;
        }
        
        size_t component_count() const { return components_.size(); }
        
        bool empty() const { return components_.empty(); }
    };

    /**
     * @brief Component validation result
     */
    struct ValidationResult {
        bool valid;
        std::vector<std::string> errors;
        std::vector<std::string> warnings;
        
        ValidationResult() : valid(true) {}
        
        void add_error(const std::string& error) {
            errors.push_back(error);
            valid = false;
        }
        
        void add_warning(const std::string& warning) {
            warnings.push_back(warning);
        }
    };

    /**
     * @brief Component event handler types
     */
    using ComponentHandler = std::function<void(const nlohmann::json&)>;
    using ButtonHandler = std::function<void(const std::string& custom_id, const nlohmann::json& interaction)>;
    using SelectHandler = std::function<void(const std::string& custom_id, const std::vector<std::string>& values, const nlohmann::json& interaction)>;
    using TextInputHandler = std::function<void(const std::string& custom_id, const std::string& value, const nlohmann::json& interaction)>;

} // namespace discord::components