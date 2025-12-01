/**
 * @file select_menu.h
 * @brief Select menu component implementations
 * 
 * Provides various select menu types including string, user, role, channel, and mentionable selects.
 */

#pragma once

#include "component_base.h"
#include <optional>
#include <vector>

namespace discord::components {

    /**
     * @brief Select menu option for string selects
     */
    struct SelectOption {
        std::string label;
        std::string value;
        std::optional<std::string> description;
        std::optional<std::string> emoji;
        bool default_ = false;

        SelectOption(const std::string& label, const std::string& value,
                    const std::optional<std::string>& description = std::nullopt,
                    const std::optional<std::string>& emoji = std::nullopt,
                    bool default_ = false)
            : label(label), value(value), description(description), emoji(emoji), default_(default_) {}

        nlohmann::json to_json() const;
    };

    /**
     * @brief Base class for select menu components
     */
    class SelectMenu : public InteractiveComponent {
    protected:
        std::string placeholder_;
        std::vector<std::string> default_values_;
        int min_values_ = 1;
        int max_values_ = 1;

    public:
        SelectMenu(const std::string& custom_id,
                  const std::string& placeholder = "",
                  int min_values = 1,
                  int max_values = 1,
                  bool disabled = false);

        virtual ~SelectMenu() = default;

        // Common methods
        void set_placeholder(const std::string& placeholder) { placeholder_ = placeholder; }
        const std::string& get_placeholder() const { return placeholder_; }

        void set_min_values(int min_values) { min_values_ = min_values; }
        int get_min_values() const { return min_values_; }

        void set_max_values(int max_values) { max_values_ = max_values; }
        int get_max_values() const { return max_values_; }

        void set_default_values(const std::vector<std::string>& values) { default_values_ = values; }
        const std::vector<std::string>& get_default_values() const { return default_values_; }

        void add_default_value(const std::string& value) { default_values_.push_back(value); }
        void clear_default_values() { default_values_.clear(); }
    };

    /**
     * @brief String select menu component
     */
    class StringSelect : public SelectMenu {
    private:
        std::vector<SelectOption> options_;

    public:
        StringSelect(const std::string& custom_id,
                    const std::vector<SelectOption>& options = {},
                    const std::string& placeholder = "",
                    int min_values = 1,
                    int max_values = 25,
                    bool disabled = false);

        ~StringSelect() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::STRING_SELECT; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // String select specific methods
        void add_option(const SelectOption& option);
        void remove_option(size_t index);
        void clear_options();
        
        const std::vector<SelectOption>& get_options() const { return options_; }
        size_t option_count() const { return options_.size(); }

        // Static factory method
        static std::unique_ptr<StringSelect> create(const std::string& custom_id,
                                                   const std::vector<SelectOption>& options = {},
                                                   const std::string& placeholder = "",
                                                   int min_values = 1,
                                                   int max_values = 25);
    };

    /**
     * @brief User select menu component
     */
    class UserSelect : public SelectMenu {
    public:
        UserSelect(const std::string& custom_id,
                  const std::string& placeholder = "",
                  int min_values = 1,
                  int max_values = 25,
                  bool disabled = false);

        ~UserSelect() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::USER_SELECT; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // Static factory method
        static std::unique_ptr<UserSelect> create(const std::string& custom_id,
                                                 const std::string& placeholder = "",
                                                 int min_values = 1,
                                                 int max_values = 25);
    };

    /**
     * @brief Role select menu component
     */
    class RoleSelect : public SelectMenu {
    public:
        RoleSelect(const std::string& custom_id,
                  const std::string& placeholder = "",
                  int min_values = 1,
                  int max_values = 25,
                  bool disabled = false);

        ~RoleSelect() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::ROLE_SELECT; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // Static factory method
        static std::unique_ptr<RoleSelect> create(const std::string& custom_id,
                                                 const std::string& placeholder = "",
                                                 int min_values = 1,
                                                 int max_values = 25);
    };

    /**
     * @brief Channel select menu component
     */
    class ChannelSelect : public SelectMenu {
    private:
        std::vector<std::string> channel_types_;

    public:
        ChannelSelect(const std::string& custom_id,
                     const std::vector<std::string>& channel_types = {},
                     const std::string& placeholder = "",
                     int min_values = 1,
                     int max_values = 25,
                     bool disabled = false);

        ~ChannelSelect() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::CHANNEL_SELECT; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // Channel select specific methods
        void add_channel_type(const std::string& type);
        void remove_channel_type(const std::string& type);
        void clear_channel_types();
        
        const std::vector<std::string>& get_channel_types() const { return channel_types_; }

        // Static factory method
        static std::unique_ptr<ChannelSelect> create(const std::string& custom_id,
                                                   const std::vector<std::string>& channel_types = {},
                                                   const std::string& placeholder = "",
                                                   int min_values = 1,
                                                   int max_values = 25);
    };

    /**
     * @brief Mentionable select menu component (users and roles)
     */
    class MentionableSelect : public SelectMenu {
    public:
        MentionableSelect(const std::string& custom_id,
                         const std::string& placeholder = "",
                         int min_values = 1,
                         int max_values = 25,
                         bool disabled = false);

        ~MentionableSelect() override = default;

        // IComponent interface
        ComponentType get_type() const override { return ComponentType::MENTIONABLE_SELECT; }
        nlohmann::json to_json() const override;
        bool validate() const override;
        std::unique_ptr<IComponent> clone() const override;

        // Static factory method
        static std::unique_ptr<MentionableSelect> create(const std::string& custom_id,
                                                        const std::string& placeholder = "",
                                                        int min_values = 1,
                                                        int max_values = 25);
    };

} // namespace discord::components