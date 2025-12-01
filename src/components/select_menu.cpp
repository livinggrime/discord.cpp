#include "../include/discord/components/select_menu.h"
#include "../include/discord/utils/types.h"
#include <stdexcept>
#include <algorithm>

namespace discord::components {

    nlohmann::json SelectOption::to_json() const {
        nlohmann::json json;
        json["label"] = label;
        json["value"] = value;
        
        if (description.has_value()) {
            json["description"] = description.value();
        }
        
        if (emoji.has_value()) {
            json["emoji"] = nlohmann::json{{"name", emoji.value()}};
        }
        
        if (default_) {
            json["default"] = true;
        }
        
        return json;
    }

    SelectMenu::SelectMenu(const std::string& custom_id,
                           const std::string& placeholder,
                           int min_values,
                           int max_values,
                           bool disabled)
        : InteractiveComponent(custom_id, disabled),
          placeholder_(placeholder),
          min_values_(min_values),
          max_values_(max_values) {
        
        if (custom_id_.length() > 100) {
            throw std::invalid_argument("Select menu custom_id cannot exceed 100 characters");
        }
        
        if (min_values_ < 0 || min_values_ > 25) {
            throw std::invalid_argument("min_values must be between 0 and 25");
        }
        
        if (max_values_ < 1 || max_values_ > 25) {
            throw std::invalid_argument("max_values must be between 1 and 25");
        }
        
        if (min_values_ > max_values_) {
            throw std::invalid_argument("min_values cannot be greater than max_values");
        }
    }

    void SelectMenu::set_default_values(const std::vector<std::string>& values) {
        if (values.size() > static_cast<size_t>(max_values_)) {
            throw std::invalid_argument("Default values count cannot exceed max_values");
        }
        default_values_ = values;
    }

    void SelectMenu::add_default_value(const std::string& value) {
        if (default_values_.size() >= static_cast<size_t>(max_values_)) {
            throw std::invalid_argument("Cannot add more default values than max_values allows");
        }
        default_values_.push_back(value);
    }

    StringSelect::StringSelect(const std::string& custom_id,
                             const std::vector<SelectOption>& options,
                             const std::string& placeholder,
                             int min_values,
                             int max_values,
                             bool disabled)
        : SelectMenu(custom_id, placeholder, min_values, max_values, disabled),
          options_(options) {
        
        if (options_.size() > 25) {
            throw std::invalid_argument("String select cannot have more than 25 options");
        }
        
        if (options_.empty()) {
            throw std::invalid_argument("String select must have at least one option");
        }
    }

    nlohmann::json StringSelect::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::STRING_SELECT);
        json["custom_id"] = custom_id_;
        
        if (!placeholder_.empty()) {
            json["placeholder"] = placeholder_;
        }
        
        json["min_values"] = min_values_;
        json["max_values"] = max_values_;
        
        nlohmann::json options_array = nlohmann::json::array();
        for (const auto& option : options_) {
            options_array.push_back(option.to_json());
        }
        json["options"] = options_array;
        
        if (!default_values_.empty()) {
            json["default_values"] = default_values_;
        }
        
        if (disabled_) {
            json["disabled"] = true;
        }
        
        return json;
    }

    bool StringSelect::validate() const {
        if (custom_id_.empty() || custom_id_.length() > 100) {
            return false;
        }
        
        if (options_.empty() || options_.size() > 25) {
            return false;
        }
        
        if (min_values_ < 0 || min_values_ > 25) {
            return false;
        }
        
        if (max_values_ < 1 || max_values_ > 25) {
            return false;
        }
        
        if (min_values_ > max_values_) {
            return false;
        }
        
        if (default_values_.size() > static_cast<size_t>(max_values_)) {
            return false;
        }
        
        // Validate each option
        for (const auto& option : options_) {
            if (option.label.empty() || option.label.length() > 100) {
                return false;
            }
            if (option.value.empty() || option.value.length() > 100) {
                return false;
            }
        }
        
        return true;
    }

    std::unique_ptr<IComponent> StringSelect::clone() const {
        return std::make_unique<StringSelect>(custom_id_, options_, placeholder_, 
                                          min_values_, max_values_, disabled_);
    }

    void StringSelect::add_option(const SelectOption& option) {
        if (options_.size() >= 25) {
            throw std::invalid_argument("String select cannot have more than 25 options");
        }
        options_.push_back(option);
    }

    void StringSelect::remove_option(size_t index) {
        if (index < options_.size()) {
            options_.erase(options_.begin() + index);
        }
    }

    void StringSelect::clear_options() {
        options_.clear();
    }

    std::unique_ptr<StringSelect> StringSelect::create(const std::string& custom_id,
                                                    const std::vector<SelectOption>& options,
                                                    const std::string& placeholder,
                                                    int min_values,
                                                    int max_values) {
        return std::make_unique<StringSelect>(custom_id, options, placeholder, min_values, max_values);
    }

    UserSelect::UserSelect(const std::string& custom_id,
                           const std::string& placeholder,
                           int min_values,
                           int max_values,
                           bool disabled)
        : SelectMenu(custom_id, placeholder, min_values, max_values, disabled) {}

    nlohmann::json UserSelect::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::USER_SELECT);
        json["custom_id"] = custom_id_;
        
        if (!placeholder_.empty()) {
            json["placeholder"] = placeholder_;
        }
        
        json["min_values"] = min_values_;
        json["max_values"] = max_values_;
        
        if (!default_values_.empty()) {
            json["default_values"] = default_values_;
        }
        
        if (disabled_) {
            json["disabled"] = true;
        }
        
        return json;
    }

    bool UserSelect::validate() const {
        if (custom_id_.empty() || custom_id_.length() > 100) {
            return false;
        }
        
        if (min_values_ < 0 || min_values_ > 25) {
            return false;
        }
        
        if (max_values_ < 1 || max_values_ > 25) {
            return false;
        }
        
        if (min_values_ > max_values_) {
            return false;
        }
        
        if (default_values_.size() > static_cast<size_t>(max_values_)) {
            return false;
        }
        
        return true;
    }

    std::unique_ptr<IComponent> UserSelect::clone() const {
        return std::make_unique<UserSelect>(custom_id_, placeholder_, 
                                         min_values_, max_values_, disabled_);
    }

    std::unique_ptr<UserSelect> UserSelect::create(const std::string& custom_id,
                                                  const std::string& placeholder,
                                                  int min_values,
                                                  int max_values) {
        return std::make_unique<UserSelect>(custom_id, placeholder, min_values, max_values);
    }

    RoleSelect::RoleSelect(const std::string& custom_id,
                           const std::string& placeholder,
                           int min_values,
                           int max_values,
                           bool disabled)
        : SelectMenu(custom_id, placeholder, min_values, max_values, disabled) {}

    nlohmann::json RoleSelect::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::ROLE_SELECT);
        json["custom_id"] = custom_id_;
        
        if (!placeholder_.empty()) {
            json["placeholder"] = placeholder_;
        }
        
        json["min_values"] = min_values_;
        json["max_values"] = max_values_;
        
        if (!default_values_.empty()) {
            json["default_values"] = default_values_;
        }
        
        if (disabled_) {
            json["disabled"] = true;
        }
        
        return json;
    }

    bool RoleSelect::validate() const {
        if (custom_id_.empty() || custom_id_.length() > 100) {
            return false;
        }
        
        if (min_values_ < 0 || min_values_ > 25) {
            return false;
        }
        
        if (max_values_ < 1 || max_values_ > 25) {
            return false;
        }
        
        if (min_values_ > max_values_) {
            return false;
        }
        
        if (default_values_.size() > static_cast<size_t>(max_values_)) {
            return false;
        }
        
        return true;
    }

    std::unique_ptr<IComponent> RoleSelect::clone() const {
        return std::make_unique<RoleSelect>(custom_id_, placeholder_, 
                                         min_values_, max_values_, disabled_);
    }

    std::unique_ptr<RoleSelect> RoleSelect::create(const std::string& custom_id,
                                                  const std::string& placeholder,
                                                  int min_values,
                                                  int max_values) {
        return std::make_unique<RoleSelect>(custom_id, placeholder, min_values, max_values);
    }

    ChannelSelect::ChannelSelect(const std::string& custom_id,
                               const std::vector<std::string>& channel_types,
                               const std::string& placeholder,
                               int min_values,
                               int max_values,
                               bool disabled)
        : SelectMenu(custom_id, placeholder, min_values, max_values, disabled),
          channel_types_(channel_types) {}

    nlohmann::json ChannelSelect::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::CHANNEL_SELECT);
        json["custom_id"] = custom_id_;
        
        if (!placeholder_.empty()) {
            json["placeholder"] = placeholder_;
        }
        
        json["min_values"] = min_values_;
        json["max_values"] = max_values_;
        
        if (!channel_types_.empty()) {
            json["channel_types"] = channel_types_;
        }
        
        if (!default_values_.empty()) {
            json["default_values"] = default_values_;
        }
        
        if (disabled_) {
            json["disabled"] = true;
        }
        
        return json;
    }

    bool ChannelSelect::validate() const {
        if (custom_id_.empty() || custom_id_.length() > 100) {
            return false;
        }
        
        if (min_values_ < 0 || min_values_ > 25) {
            return false;
        }
        
        if (max_values_ < 1 || max_values_ > 25) {
            return false;
        }
        
        if (min_values_ > max_values_) {
            return false;
        }
        
        if (default_values_.size() > static_cast<size_t>(max_values_)) {
            return false;
        }
        
        return true;
    }

    std::unique_ptr<IComponent> ChannelSelect::clone() const {
        return std::make_unique<ChannelSelect>(custom_id_, channel_types_, placeholder_,
                                            min_values_, max_values_, disabled_);
    }

    void ChannelSelect::add_channel_type(const std::string& type) {
        channel_types_.push_back(type);
    }

    void ChannelSelect::remove_channel_type(const std::string& type) {
        auto it = std::find(channel_types_.begin(), channel_types_.end(), type);
        if (it != channel_types_.end()) {
            channel_types_.erase(it);
        }
    }

    void ChannelSelect::clear_channel_types() {
        channel_types_.clear();
    }

    std::unique_ptr<ChannelSelect> ChannelSelect::create(const std::string& custom_id,
                                                    const std::vector<std::string>& channel_types,
                                                    const std::string& placeholder,
                                                    int min_values,
                                                    int max_values) {
        return std::make_unique<ChannelSelect>(custom_id, channel_types, placeholder, min_values, max_values);
    }

    MentionableSelect::MentionableSelect(const std::string& custom_id,
                                       const std::string& placeholder,
                                       int min_values,
                                       int max_values,
                                       bool disabled)
        : SelectMenu(custom_id, placeholder, min_values, max_values, disabled) {}

    nlohmann::json MentionableSelect::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::MENTIONABLE_SELECT);
        json["custom_id"] = custom_id_;
        
        if (!placeholder_.empty()) {
            json["placeholder"] = placeholder_;
        }
        
        json["min_values"] = min_values_;
        json["max_values"] = max_values_;
        
        if (!default_values_.empty()) {
            json["default_values"] = default_values_;
        }
        
        if (disabled_) {
            json["disabled"] = true;
        }
        
        return json;
    }

    bool MentionableSelect::validate() const {
        if (custom_id_.empty() || custom_id_.length() > 100) {
            return false;
        }
        
        if (min_values_ < 0 || min_values_ > 25) {
            return false;
        }
        
        if (max_values_ < 1 || max_values_ > 25) {
            return false;
        }
        
        if (min_values_ > max_values_) {
            return false;
        }
        
        if (default_values_.size() > static_cast<size_t>(max_values_)) {
            return false;
        }
        
        return true;
    }

    std::unique_ptr<IComponent> MentionableSelect::clone() const {
        return std::make_unique<MentionableSelect>(custom_id_, placeholder_,
                                                 min_values_, max_values_, disabled_);
    }

    std::unique_ptr<MentionableSelect> MentionableSelect::create(const std::string& custom_id,
                                                            const std::string& placeholder,
                                                            int min_values,
                                                            int max_values) {
        return std::make_unique<MentionableSelect>(custom_id, placeholder, min_values, max_values);
    }

} // namespace discord::components