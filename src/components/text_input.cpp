#include "../include/discord/components/text_input.h"
#include "../include/discord/utils/types.h"
#include <stdexcept>

namespace discord::components {

    TextInput::TextInput(const std::string& custom_id,
                             const std::string& label,
                             TextInputStyle style,
                             const std::optional<std::string>& value,
                             const std::optional<std::string>& placeholder,
                             bool required,
                             int min_length,
                             int max_length)
        : InteractiveComponent(custom_id, false),
          style_(style),
          label_(label),
          value_(value),
          placeholder_(placeholder),
          required_(required),
          min_length_(min_length),
          max_length_(max_length) {
        
        if (custom_id_.empty() || custom_id_.length() > 100) {
            throw std::invalid_argument("TextInput custom_id must be 1-100 characters");
        }
        
        if (label_.empty() || label_.length() > 45) {
            throw std::invalid_argument("TextInput label must be 1-45 characters");
        }
        
        if (min_length_ < 0 || min_length_ > 4000) {
            throw std::invalid_argument("TextInput min_length must be 0-4000");
        }
        
        if (max_length_ < 1 || max_length_ > 4000) {
            throw std::invalid_argument("TextInput max_length must be 1-4000");
        }
        
        if (min_length_ > max_length_) {
            throw std::invalid_argument("TextInput min_length cannot be greater than max_length");
        }
        
        if (value_.has_value() && value_->length() > static_cast<size_t>(max_length_)) {
            throw std::invalid_argument("TextInput value length cannot exceed max_length");
        }
    }

    nlohmann::json TextInput::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::TEXT_INPUT);
        json["custom_id"] = custom_id_;
        json["style"] = static_cast<int>(style_);
        json["label"] = label_;
        
        if (value_.has_value()) {
            json["value"] = value_.value();
        }
        
        if (placeholder_.has_value()) {
            json["placeholder"] = placeholder_.value();
        }
        
        if (required_) {
            json["required"] = true;
        }
        
        json["min_length"] = min_length_;
        json["max_length"] = max_length_;
        
        return json;
    }

    bool TextInput::validate() const {
        if (custom_id_.empty() || custom_id_.length() > 100) {
            return false;
        }
        
        if (label_.empty() || label_.length() > 45) {
            return false;
        }
        
        if (min_length_ < 0 || min_length_ > 4000) {
            return false;
        }
        
        if (max_length_ < 1 || max_length_ > 4000) {
            return false;
        }
        
        if (min_length_ > max_length_) {
            return false;
        }
        
        if (value_.has_value() && value_->length() > static_cast<size_t>(max_length_)) {
            return false;
        }
        
        return true;
    }

    std::unique_ptr<IComponent> TextInput::clone() const {
        return std::make_unique<TextInput>(custom_id_, label_, style_, value_, 
                                       placeholder_, required_, min_length_, max_length_);
    }

    std::unique_ptr<TextInput> TextInput::short_text(const std::string& custom_id,
                                                     const std::string& label,
                                                     const std::optional<std::string>& value,
                                                     const std::optional<std::string>& placeholder,
                                                     bool required,
                                                     int min_length,
                                                     int max_length) {
        return std::make_unique<TextInput>(custom_id, label, TextInputStyle::SHORT, 
                                       value, placeholder, required, min_length, max_length);
    }

    std::unique_ptr<TextInput> TextInput::paragraph(const std::string& custom_id,
                                                  const std::string& label,
                                                  const std::optional<std::string>& value,
                                                  const std::optional<std::string>& placeholder,
                                                  bool required,
                                                  int min_length,
                                                  int max_length) {
        return std::make_unique<TextInput>(custom_id, label, TextInputStyle::PARAGRAPH, 
                                       value, placeholder, required, min_length, max_length);
    }

    std::unique_ptr<TextInput> TextInput::create(const std::string& custom_id,
                                               const std::string& label,
                                               TextInputStyle style,
                                               const std::optional<std::string>& value,
                                               const std::optional<std::string>& placeholder,
                                               bool required,
                                               int min_length,
                                               int max_length) {
        return std::make_unique<TextInput>(custom_id, label, style, value, 
                                       placeholder, required, min_length, max_length);
    }

} // namespace discord::components