#include "../include/discord/components/section.h"
#include "../include/discord/components/button.h"
#include "../include/discord/components/select_menu.h"
#include "../include/discord/components/text_input.h"
#include "../include/discord/utils/types.h"

namespace discord::components {

    Section::Section(const std::string& text,
                         const std::optional<std::shared_ptr<IComponent>>& accessory)
        : ContainerComponent(),
          text_(text),
          accessory_(accessory) {
        
        if (text_.length() > 2000) {
            throw std::invalid_argument("Section text cannot exceed 2000 characters");
        }
        
        if (accessory_.has_value()) {
            add_component(accessory_.value());
        }
    }

    void Section::set_text(const std::string& text) {
        if (text.length() > 2000) {
            throw std::invalid_argument("Section text cannot exceed 2000 characters");
        }
        text_ = text;
    }

    void Section::set_accessory(const std::optional<std::shared_ptr<IComponent>>& accessory) {
        accessory_ = accessory;
        
        // Update components list
        components_.clear();
        if (accessory_.has_value()) {
            components_.push_back(accessory_.value());
        }
    }

    nlohmann::json Section::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::SECTION);
        json["text"] = text_;
        
        if (accessory_.has_value() && accessory_.value()) {
            json["accessory"] = accessory_.value()->to_json();
        }
        
        return json;
    }

    bool Section::validate() const {
        if (text_.empty() || text_.length() > 2000) {
            return false;
        }
        
        if (accessory_.has_value()) {
            if (!accessory_.value()) {
                return false;
            }
            
            // Validate that accessory is a valid component type for sections
            ComponentType accessory_type = accessory_.value()->get_type();
            if (accessory_type != ComponentType::BUTTON && 
                accessory_type != ComponentType::STRING_SELECT &&
                accessory_type != ComponentType::USER_SELECT &&
                accessory_type != ComponentType::ROLE_SELECT &&
                accessory_type != ComponentType::CHANNEL_SELECT &&
                accessory_type != ComponentType::MENTIONABLE_SELECT &&
                accessory_type != ComponentType::TEXT_INPUT) {
                return false;
            }
            
            return accessory_.value()->validate();
        }
        
        return true;
    }

    std::unique_ptr<IComponent> Section::clone() const {
        std::optional<std::shared_ptr<IComponent>> cloned_accessory;
        if (accessory_.has_value() && accessory_.value()) {
            cloned_accessory = std::shared_ptr<IComponent>(accessory_.value()->clone().release());
        }
        
        return std::make_unique<Section>(text_, cloned_accessory);
    }

    std::unique_ptr<Section> Section::create(const std::string& text,
                                               const std::optional<std::shared_ptr<IComponent>>& accessory) {
        return std::make_unique<Section>(text, accessory);
    }

    std::unique_ptr<Section> Section::with_button(const std::string& text,
                                                   std::shared_ptr<Button> button) {
        std::optional<std::shared_ptr<IComponent>> accessory;
        if (button) {
            accessory = std::static_pointer_cast<IComponent>(button);
        }
        return std::make_unique<Section>(text, accessory);
    }

    std::unique_ptr<Section> Section::with_select(const std::string& text,
                                                  std::shared_ptr<SelectMenu> select) {
        std::optional<std::shared_ptr<IComponent>> accessory;
        if (select) {
            accessory = std::static_pointer_cast<IComponent>(select);
        }
        return std::make_unique<Section>(text, accessory);
    }

    std::unique_ptr<Section> Section::with_text_input(const std::string& text,
                                                       std::shared_ptr<TextInput> text_input) {
        std::optional<std::shared_ptr<IComponent>> accessory;
        if (text_input) {
            accessory = std::static_pointer_cast<IComponent>(text_input);
        }
        return std::make_unique<Section>(text, accessory);
    }

} // namespace discord::components