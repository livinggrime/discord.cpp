/**
 * @file component_utils.h
 * @brief Utility functions and helpers for components
 * 
 * Provides various utility functions for working with components.
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
#include <vector>
#include <string>
#include <optional>
#include <functional>
#include <map>

namespace discord::components {

    /**
     * @brief Utility functions for component operations
     */
    class ComponentUtils {
    public:
        // Component type utilities
        static std::string component_type_to_string(ComponentType type);
        static ComponentType string_to_component_type(const std::string& type_str);
        static std::vector<ComponentType> get_all_component_types();
        static bool is_interactive_type(ComponentType type);
        static bool is_container_type(ComponentType type);
        static bool is_content_type(ComponentType type);
        
        // Button style utilities
        static std::string button_style_to_string(ButtonStyle style);
        static ButtonStyle string_to_button_style(const std::string& style_str);
        static std::vector<ButtonStyle> get_all_button_styles();
        
        // Text input style utilities
        static std::string text_input_style_to_string(TextInputStyle style);
        static TextInputStyle string_to_text_input_style(const std::string& style_str);
        static std::vector<TextInputStyle> get_all_text_input_styles();
        
        // Component search and filtering
        static std::vector<std::shared_ptr<IComponent>> find_components_by_type(
            const std::vector<std::shared_ptr<IComponent>>& components,
            ComponentType type);
        
        static std::vector<std::shared_ptr<IComponent>> find_components_by_custom_id(
            const std::vector<std::shared_ptr<IComponent>>& components,
            const std::string& custom_id);
        
        static std::shared_ptr<IComponent> find_component_by_custom_id(
            const std::vector<std::shared_ptr<IComponent>>& components,
            const std::string& custom_id);
        
        static std::vector<std::shared_ptr<IComponent>> filter_components(
            const std::vector<std::shared_ptr<IComponent>>& components,
            const std::function<bool(const IComponent&)>& predicate);
        
        // Component validation utilities
        static ValidationResult validate_component_tree(const IComponent& component);
        static ValidationResult validate_component_list(
            const std::vector<std::shared_ptr<IComponent>>& components);
        
        static bool has_valid_custom_ids(const std::vector<std::shared_ptr<IComponent>>& components);
        static std::vector<std::string> get_duplicate_custom_ids(
            const std::vector<std::shared_ptr<IComponent>>& components);
        
        // Component counting and statistics
        static size_t count_components_by_type(const IComponent& component, ComponentType type);
        static size_t count_components_by_type(
            const std::vector<std::shared_ptr<IComponent>>& components,
            ComponentType type);
        
        static std::map<ComponentType, size_t> get_component_type_counts(const IComponent& component);
        static std::map<ComponentType, size_t> get_component_type_counts(
            const std::vector<std::shared_ptr<IComponent>>& components);
        
        static size_t get_max_depth(const IComponent& component);
        static size_t get_total_component_count(const IComponent& component);
        
        // Component transformation utilities
        static std::vector<std::shared_ptr<IComponent>> flatten_component_tree(const IComponent& component);
        static std::vector<std::shared_ptr<IComponent>> flatten_component_list(
            const std::vector<std::shared_ptr<IComponent>>& components);
        
        static std::shared_ptr<IComponent> clone_component(const IComponent& component);
        static std::vector<std::shared_ptr<IComponent>> clone_components(
            const std::vector<std::shared_ptr<IComponent>>& components);
        
        // Component conversion utilities
        static nlohmann::json components_to_json_array(
            const std::vector<std::shared_ptr<IComponent>>& components);
        
        static std::vector<std::shared_ptr<IComponent>> json_array_to_components(
            const nlohmann::json& json_array);
        
        // String utilities
        static std::string escape_custom_id(const std::string& custom_id);
        static std::string unescape_custom_id(const std::string& escaped_custom_id);
        static bool is_valid_custom_id(const std::string& custom_id);
        static std::string generate_custom_id(const std::string& prefix = "");
        
        static std::string sanitize_label(const std::string& label);
        static std::string sanitize_placeholder(const std::string& placeholder);
        static bool is_valid_label(const std::string& label);
        static bool is_valid_placeholder(const std::string& placeholder);
        
        // URL and emoji utilities
        static bool is_valid_url(const std::string& url);
        static bool is_valid_emoji(const std::string& emoji);
        static std::string normalize_emoji(const std::string& emoji);
        
        // Layout utilities
        static std::vector<std::shared_ptr<ActionRow>> auto_layout_buttons(
            const std::vector<std::shared_ptr<Button>>& buttons,
            int max_per_row = 5);
        
        static std::vector<std::shared_ptr<ActionRow>> auto_layout_components(
            const std::vector<std::shared_ptr<IComponent>>& components,
            int max_per_row = 5);
        
        static std::shared_ptr<Container> create_balanced_layout(
            const std::vector<std::shared_ptr<IComponent>>& components);
        
        // Component creation helpers
        static std::shared_ptr<Button> create_confirmation_button(
            const std::string& custom_id_prefix = "");
        
        static std::shared_ptr<Button> create_cancellation_button(
            const std::string& custom_id_prefix = "");
        
        static std::shared_ptr<StringSelect> create_pagination_select(
            const std::string& custom_id,
            int current_page,
            int total_pages,
            const std::string& placeholder = "");
        
        static std::shared_ptr<TextInput> create_required_text_input(
            const std::string& custom_id,
            const std::string& label,
            TextInputStyle style = TextInputStyle::SHORT);
        
        // Component testing utilities
        static std::shared_ptr<IComponent> create_test_component(ComponentType type);
        static std::vector<std::shared_ptr<IComponent>> create_test_components();
        static std::shared_ptr<ActionRow> create_test_action_row();
        static std::shared_ptr<Section> create_test_section();
        static std::shared_ptr<Container> create_test_container();
        
        // Debug utilities
        static std::string component_to_debug_string(const IComponent& component);
        static std::string components_to_debug_string(
            const std::vector<std::shared_ptr<IComponent>>& components);
        
        static void print_component_tree(const IComponent& component, int indent = 0);
        static void print_component_list(
            const std::vector<std::shared_ptr<IComponent>>& components);
        
        // Performance utilities
        static bool is_component_tree_optimized(const IComponent& component);
        static std::shared_ptr<IComponent> optimize_component_tree(const IComponent& component);
        static std::vector<std::shared_ptr<IComponent>> optimize_component_list(
            const std::vector<std::shared_ptr<IComponent>>& components);
    };

    /**
     * @brief Component registry for type-safe component creation
     */
    class ComponentRegistry {
    private:
        static std::map<ComponentType, std::function<std::unique_ptr<IComponent>()>> creators_;
        
    public:
        template<typename T>
        static void register_component(ComponentType type) {
            creators_[type] = []() { return std::make_unique<T>(); };
        }
        
        static std::unique_ptr<IComponent> create(ComponentType type);
        static bool is_registered(ComponentType type);
        static std::vector<ComponentType> get_registered_types();
        static void initialize_defaults();
    };

    /**
     * @brief Component cache for performance optimization
     */
    class ComponentCache {
    private:
        static std::map<std::string, std::shared_ptr<IComponent>> cache_;
        static std::map<std::string, nlohmann::json> json_cache_;
        
    public:
        static void cache_component(const std::string& key, std::shared_ptr<IComponent> component);
        static std::shared_ptr<IComponent> get_cached_component(const std::string& key);
        static void remove_cached_component(const std::string& key);
        static void clear_component_cache();
        
        static void cache_json(const std::string& key, const nlohmann::json& json);
        static nlohmann::json get_cached_json(const std::string& key);
        static void remove_cached_json(const std::string& key);
        static void clear_json_cache();
        
        static void clear_all_caches();
        static size_t get_cache_size();
    };

} // namespace discord::components