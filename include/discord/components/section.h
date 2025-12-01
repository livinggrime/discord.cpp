/**
 * @file section.h
 * @brief Section container component
 * 
 * Provides section containers for organizing content with optional accessories.
 */

#pragma once

#include "component_base.h"
#include "button.h"
#include "select_menu.h"
#include "text_input.h"
#include <optional>

namespace discord::components {

    /**
     * @brief Section container component
     * 
     * Sections can contain text content and optionally one accessory component
     * (button, select menu, or text input).
     */
    class Section : public ContainerComponent {
    private:
        std::string text_;
        std::optional<std::shared_ptr<IComponent>> accessory_;

    public:
        /**
         * @brief Constructor
         */
        Section(const std::string& text,
               const std::optional<std::shared_ptr<IComponent>>& accessory = std::nullopt);

        ~Section() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::SECTION; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // Section specific methods
        void set_text(const std::string& text) { text_ = text; }
        const std::string& get_text() const { return text_; }

        void set_accessory(const std::optional<std::shared_ptr<IComponent>>& accessory);
        const std::optional<std::shared_ptr<IComponent>>& get_accessory() const { return accessory_; }
        bool has_accessory() const { return accessory_.has_value(); }

        void clear_accessory() { accessory_ = std::nullopt; }

        // Static factory method
        static std::unique_ptr<Section> create(const std::string& text,
                                             const std::optional<std::shared_ptr<IComponent>>& accessory = std::nullopt);

        // Convenience factory methods
        static std::unique_ptr<Section> with_button(const std::string& text,
                                                   std::shared_ptr<Button> button);
        
        static std::unique_ptr<Section> with_select(const std::string& text,
                                                    std::shared_ptr<SelectMenu> select);
        
        static std::unique_ptr<Section> with_text_input(const std::string& text,
                                                       std::shared_ptr<TextInput> text_input);
    };

} // namespace discord::components