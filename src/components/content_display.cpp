#include "../include/discord/components/content_display.h"
#include "../include/discord/utils/types.h"
#include <stdexcept>

namespace discord::components {

    TextDisplay::TextDisplay(const std::string& content)
        : content_(content) {
        if (content_.length() > 2000) {
            throw std::invalid_argument("TextDisplay content cannot exceed 2000 characters");
        }
    }

    nlohmann::json TextDisplay::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::TEXT_DISPLAY);
        json["text"] = content_;
        return json;
    }

    bool TextDisplay::validate() const {
        return !content_.empty() && content_.length() <= 2000;
    }

    std::unique_ptr<IComponent> TextDisplay::clone() const {
        return std::make_unique<TextDisplay>(content_);
    }

    std::unique_ptr<TextDisplay> TextDisplay::create(const std::string& content) {
        return std::make_unique<TextDisplay>(content);
    }

    Thumbnail::Thumbnail(const std::string& url,
                   const std::optional<std::string>& alt_text,
                   const std::optional<int>& width,
                   const std::optional<int>& height)
        : url_(url),
          alt_text_(alt_text),
          width_(width),
          height_(height) {
        
        if (url_.empty()) {
            throw std::invalid_argument("Thumbnail URL cannot be empty");
        }
        
        if (width_.has_value() && width_.value() < 16) {
            throw std::invalid_argument("Thumbnail width must be at least 16 pixels");
        }
        
        if (height_.has_value() && height_.value() < 16) {
            throw std::invalid_argument("Thumbnail height must be at least 16 pixels");
        }
    }

    nlohmann::json Thumbnail::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::THUMBNAIL);
        json["url"] = url_;
        
        if (alt_text_.has_value()) {
            json["alt_text"] = alt_text_.value();
        }
        
        if (width_.has_value()) {
            json["width"] = width_.value();
        }
        
        if (height_.has_value()) {
            json["height"] = height_.value();
        }
        
        return json;
    }

    bool Thumbnail::validate() const {
        if (url_.empty()) {
            return false;
        }
        
        if (width_.has_value() && width_.value() < 16) {
            return false;
        }
        
        if (height_.has_value() && height_.value() < 16) {
            return false;
        }
        
        return true;
    }

    std::unique_ptr<Thumbnail> Thumbnail::create(const std::string& url,
                                            const std::optional<std::string>& alt_text,
                                            const std::optional<int>& width,
                                            const std::optional<int>& height) {
        return std::make_unique<Thumbnail>(url, alt_text, width, height);
    }

    MediaGallery::MediaGallery(const std::vector<std::string>& items,
                           const std::optional<std::string>& alt_text)
        : items_(items),
          alt_text_(alt_text) {
        
        if (items_.size() > 10) {
            throw std::invalid_argument("MediaGallery cannot contain more than 10 items");
        }
        
        for (const auto& item : items_) {
            if (item.empty()) {
                throw std::invalid_argument("MediaGallery items cannot be empty");
            }
        }
    }

    nlohmann::json MediaGallery::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::MEDIA_GALLERY);
        json["items"] = items_;
        
        if (alt_text_.has_value()) {
            json["alt_text"] = alt_text_.value();
        }
        
        return json;
    }

    bool MediaGallery::validate() const {
        if (items_.empty() || items_.size() > 10) {
            return false;
        }
        
        for (const auto& item : items_) {
            if (item.empty()) {
                return false;
            }
        }
        
        return true;
    }

    void MediaGallery::add_item(const std::string& item) {
        if (items_.size() >= 10) {
            throw std::invalid_argument("MediaGallery cannot contain more than 10 items");
        }
        
        if (item.empty()) {
            throw std::invalid_argument("MediaGallery items cannot be empty");
        }
        
        items_.push_back(item);
    }

    void MediaGallery::remove_item(size_t index) {
        if (index < items_.size()) {
            items_.erase(items_.begin() + index);
        }
    }

    void MediaGallery::clear_items() {
        items_.clear();
    }

    std::unique_ptr<MediaGallery> MediaGallery::create(const std::vector<std::string>& items,
                                                const std::optional<std::string>& alt_text) {
        return std::make_unique<MediaGallery>(items, alt_text);
    }

    File::File(const std::string& url,
               const std::string& filename,
               const std::optional<int>& size_bytes)
        : url_(url),
          filename_(filename),
          size_bytes_(size_bytes) {
        
        if (url_.empty()) {
            throw std::invalid_argument("File URL cannot be empty");
        }
        
        if (filename_.empty()) {
            throw std::invalid_argument("File filename cannot be empty");
        }
        
        if (size_bytes_.has_value() && size_bytes_.value() < 0) {
            throw std::invalid_argument("File size_bytes must be non-negative");
        }
    }

    nlohmann::json File::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::FILE);
        json["url"] = url_;
        json["filename"] = filename_;
        
        if (size_bytes_.has_value()) {
            json["size_bytes"] = size_bytes_.value();
        }
        
        return json;
    }

    bool File::validate() const {
        if (url_.empty() || filename_.empty()) {
            return false;
        }
        
        if (size_bytes_.has_value() && size_bytes_.value() < 0) {
            return false;
        }
        
        return true;
    }

    std::unique_ptr<File> File::create(const std::string& url,
                                       const std::string& filename,
                                       const std::optional<int>& size_bytes) {
        return std::make_unique<File>(url, filename, size_bytes);
    }

    Separator::Separator(bool decorative, const std::optional<int>& spacing)
        : decorative_(decorative),
          spacing_(spacing) {
        
        if (spacing_.has_value() && spacing_.value() < 0) {
            throw std::invalid_argument("Separator spacing must be non-negative");
        }
    }

    nlohmann::json Separator::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::SEPARATOR);
        
        if (decorative_) {
            json["decorative"] = true;
        }
        
        if (spacing_.has_value()) {
            json["spacing"] = spacing_.value();
        }
        
        return json;
    }

    bool Separator::validate() const {
        if (spacing_.has_value() && spacing_.value() < 0) {
            return false;
        }
        
        return true;
    }

    std::unique_ptr<Separator> Separator::create(bool decorative, const std::optional<int>& spacing) {
        return std::make_unique<Separator>(decorative, spacing);
    }

    Label::Label(const std::string& text,
                 const std::optional<std::string>& for_component)
        : text_(text),
          for_component_(for_component) {
        
        if (text_.length() > 80) {
            throw std::invalid_argument("Label text cannot exceed 80 characters");
        }
    }

    nlohmann::json Label::to_json() const {
        nlohmann::json json;
        json["type"] = static_cast<int>(ComponentType::LABEL);
        json["text"] = text_;
        
        if (for_component_.has_value()) {
            json["for_component"] = for_component_.value();
        }
        
        return json;
    }

    bool Label::validate() const {
        if (text_.empty() || text_.length() > 80) {
            return false;
        }
        
        return true;
    }

    std::unique_ptr<Label> Label::create(const std::string& text,
                                       const std::optional<std::string>& for_component) {
        return std::make_unique<Label>(text, for_component);
    }

} // namespace discord::components