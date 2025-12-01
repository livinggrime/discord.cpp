/**
 * @file embed_components.h
 * @brief Embed-specific components and utilities
 * 
 * Provides components specifically designed for embed integration.
 */

#pragma once

#include "component_base.h"
#include "button.h"
#include "select_menu.h"
#include "text_input.h"
#include "action_row.h"
#include "section.h"
#include <optional>

namespace discord::components {

    /**
     * @brief Embed component wrapper for integrating components with embeds
     */
    class EmbedComponent : public IComponent {
    private:
        std::shared_ptr<IComponent> component_;
        std::optional<std::string> embed_id_;

    public:
        EmbedComponent(std::shared_ptr<IComponent> component,
                      const std::optional<std::string>& embed_id = std::nullopt);

        ~EmbedComponent() override = default;

        ComponentType get_type() const override;
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        void set_component(std::shared_ptr<IComponent> component) { component_ = component; }
        std::shared_ptr<IComponent> get_component() const { return component_; }

        void set_embed_id(const std::optional<std::string>& embed_id) { embed_id_ = embed_id; }
        const std::optional<std::string>& get_embed_id() const { return embed_id_; }

        static std::unique_ptr<EmbedComponent> create(std::shared_ptr<IComponent> component,
                                                       const std::optional<std::string>& embed_id = std::nullopt);
    };

    /**
     * @brief Embed layout manager for organizing components within embeds
     */
    class EmbedLayoutManager {
    private:
        std::vector<std::shared_ptr<EmbedComponent>> components_;
        std::string embed_id_;

    public:
        explicit EmbedLayoutManager(const std::string& embed_id);

        void add_component(std::shared_ptr<IComponent> component,
                           const std::optional<std::string>& component_embed_id = std::nullopt);

        void remove_component(size_t index);
        void clear_components();

        const std::vector<std::shared_ptr<EmbedComponent>>& get_components() const { return components_; }
        size_t component_count() const { return components_.size(); }

        nlohmann::json to_json() const;
        bool validate() const;

        const std::string& get_embed_id() const { return embed_id_; }
        void set_embed_id(const std::string& embed_id) { embed_id_ = embed_id; }

        // Convenience methods
        void add_button_to_embed(std::shared_ptr<Button> button,
                                 const std::optional<std::string>& component_embed_id = std::nullopt);

        void add_select_to_embed(std::shared_ptr<SelectMenu> select,
                                 const std::optional<std::string>& component_embed_id = std::nullopt);

        void add_text_input_to_embed(std::shared_ptr<TextInput> text_input,
                                     const std::optional<std::string>& component_embed_id = std::nullopt);

        static std::unique_ptr<EmbedLayoutManager> create(const std::string& embed_id);
    };

    /**
     * @brief Embed component factory for creating embed-compatible components
     */
    class EmbedComponentFactory {
    public:
        static std::unique_ptr<EmbedComponent> wrap_button(std::shared_ptr<Button> button,
                                                           const std::optional<std::string>& embed_id = std::nullopt);

        static std::unique_ptr<EmbedComponent> wrap_select(std::shared_ptr<SelectMenu> select,
                                                           const std::optional<std::string>& embed_id = std::nullopt);

        static std::unique_ptr<EmbedComponent> wrap_text_input(std::shared_ptr<TextInput> text_input,
                                                               const std::optional<std::string>& embed_id = std::nullopt);

        static std::unique_ptr<EmbedComponent> wrap_action_row(std::shared_ptr<ActionRow> action_row,
                                                               const std::optional<std::string>& embed_id = std::nullopt);

        static std::unique_ptr<EmbedComponent> wrap_section(std::shared_ptr<Section> section,
                                                           const std::optional<std::string>& embed_id = std::nullopt);
    };

} // namespace discord::components