/**
 * @file content_display.h
 * @brief Content display components (text, thumbnails, media, files, separators, labels)
 * 
 * Provides non-interactive content display components for rich message layouts.
 */

#pragma once

#include "component_base.h"
#include <optional>

namespace discord::components {

    /**
     * @brief Text display component
     */
    class TextDisplay : public IComponent {
    private:
        std::string content_;

    public:
        explicit TextDisplay(const std::string& content);
        ~TextDisplay() override = default;

        ComponentType get_type() const override { return ComponentType::TEXT_DISPLAY; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        void set_content(const std::string& content) { content_ = content; }
        const std::string& get_content() const { return content_; }

        static std::unique_ptr<TextDisplay> create(const std::string& content);
    };

    /**
     * @brief Thumbnail component
     */
    class Thumbnail : public IComponent {
    private:
        std::string url_;
        std::optional<std::string> alt_text_;
        std::optional<int> width_;
        std::optional<int> height_;

    public:
        Thumbnail(const std::string& url,
                 const std::optional<std::string>& alt_text = std::nullopt,
                 const std::optional<int>& width = std::nullopt,
                 const std::optional<int>& height = std::nullopt);

        ~Thumbnail() override = default;

        ComponentType get_type() const override { return ComponentType::THUMBNAIL; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        void set_url(const std::string& url) { url_ = url; }
        const std::string& get_url() const { return url_; }

        void set_alt_text(const std::optional<std::string>& alt_text) { alt_text_ = alt_text; }
        const std::optional<std::string>& get_alt_text() const { return alt_text_; }

        void set_width(const std::optional<int>& width) { width_ = width; }
        const std::optional<int>& get_width() const { return width_; }

        void set_height(const std::optional<int>& height) { height_ = height; }
        const std::optional<int>& get_height() const { return height_; }

        static std::unique_ptr<Thumbnail> create(const std::string& url,
                                                const std::optional<std::string>& alt_text = std::nullopt,
                                                const std::optional<int>& width = std::nullopt,
                                                const std::optional<int>& height = std::nullopt);
    };

    /**
     * @brief Media gallery component
     */
    class MediaGallery : public IComponent {
    private:
        std::vector<std::string> items_;
        std::optional<std::string> alt_text_;

    public:
        MediaGallery(const std::vector<std::string>& items,
                    const std::optional<std::string>& alt_text = std::nullopt);

        ~MediaGallery() override = default;

        ComponentType get_type() const override { return ComponentType::MEDIA_GALLERY; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        void add_item(const std::string& item);
        void remove_item(size_t index);
        void clear_items();

        const std::vector<std::string>& get_items() const { return items_; }
        size_t item_count() const { return items_.size(); }

        void set_alt_text(const std::optional<std::string>& alt_text) { alt_text_ = alt_text; }
        const std::optional<std::string>& get_alt_text() const { return alt_text_; }

        static std::unique_ptr<MediaGallery> create(const std::vector<std::string>& items,
                                                   const std::optional<std::string>& alt_text = std::nullopt);
    };

    /**
     * @brief File component
     */
    class File : public IComponent {
    private:
        std::string url_;
        std::string filename_;
        std::optional<int> size_bytes_;

    public:
        File(const std::string& url,
             const std::string& filename,
             const std::optional<int>& size_bytes = std::nullopt);

        ~File() override = default;

        ComponentType get_type() const override { return ComponentType::FILE; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        void set_url(const std::string& url) { url_ = url; }
        const std::string& get_url() const { return url_; }

        void set_filename(const std::string& filename) { filename_ = filename; }
        const std::string& get_filename() const { return filename_; }

        void set_size_bytes(const std::optional<int>& size_bytes) { size_bytes_ = size_bytes; }
        const std::optional<int>& get_size_bytes() const { return size_bytes_; }

        static std::unique_ptr<File> create(const std::string& url,
                                           const std::string& filename,
                                           const std::optional<int>& size_bytes = std::nullopt);
    };

    /**
     * @brief Separator component
     */
    class Separator : public IComponent {
    private:
        bool decorative_ = true;
        std::optional<int> spacing_;

    public:
        Separator(bool decorative = true, const std::optional<int>& spacing = std::nullopt);
        ~Separator() override = default;

        ComponentType get_type() const override { return ComponentType::SEPARATOR; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        void set_decorative(bool decorative) { decorative_ = decorative; }
        bool is_decorative() const { return decorative_; }

        void set_spacing(const std::optional<int>& spacing) { spacing_ = spacing; }
        const std::optional<int>& get_spacing() const { return spacing_; }

        static std::unique_ptr<Separator> create(bool decorative = true,
                                                const std::optional<int>& spacing = std::nullopt);
    };

    /**
     * @brief Label component
     */
    class Label : public IComponent {
    private:
        std::string text_;
        std::optional<std::string> for_component_;

    public:
        Label(const std::string& text, const std::optional<std::string>& for_component = std::nullopt);
        ~Label() override = default;

        ComponentType get_type() const override { return ComponentType::LABEL; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        void set_text(const std::string& text) { text_ = text; }
        const std::string& get_text() const { return text_; }

        void set_for_component(const std::optional<std::string>& for_component) { for_component_ = for_component; }
        const std::optional<std::string>& get_for_component() const { return for_component_; }

        static std::unique_ptr<Label> create(const std::string& text,
                                            const std::optional<std::string>& for_component = std::nullopt);
    };

} // namespace discord::components