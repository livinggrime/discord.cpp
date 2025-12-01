/**
 * @file container.h
 * @brief Container component for organizing other components
 * 
 * Provides flexible container components for organizing layouts.
 */

#pragma once

#include "component_base.h"
#include "section.h"
#include "action_row.h"

namespace discord::components {

    /**
     * @brief Container component for organizing other components
     * 
     * Containers can hold any type of component and provide flexible layout options.
     */
    class Container : public ContainerComponent {
    public:
        /**
         * @brief Constructor
         */
        Container(const std::vector<std::shared_ptr<IComponent>>& components = {});

        ~Container() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::CONTAINER; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // Container specific methods
        bool can_add_component(std::shared_ptr<IComponent> component) const;
        size_t max_components() const { return 10; } // Discord limit for containers
        bool is_full() const { return components_.size() >= max_components(); }

        // Static factory method
        static std::unique_ptr<Container> create(const std::vector<std::shared_ptr<IComponent>>& components = {});

        // Convenience factory methods
        static std::unique_ptr<Container> with_sections(const std::vector<std::shared_ptr<Section>>& sections);
        static std::unique_ptr<Container> with_action_rows(const std::vector<std::shared_ptr<ActionRow>>& action_rows);
        static std::unique_ptr<Container> mixed(const std::vector<std::shared_ptr<IComponent>>& components);
    };

} // namespace discord::components