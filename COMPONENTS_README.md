# Discord.cpp Components System

A comprehensive, type-safe, and modular C++ library for creating Discord UI components. This system supports all Discord component types including buttons, select menus, text inputs, and the latest Components V2 features.

## 🚀 Features

### Component Types
- **Interactive Components**: Buttons, Select Menus, Text Inputs
- **Container Components**: Action Rows, Sections, Containers  
- **Content Display**: Text, Thumbnails, Media Galleries, Files, Separators, Labels
- **Layout Components**: Auto-layout utilities and responsive design

### Key Features
- ✅ **Type Safety**: Full C++ type safety with compile-time checks
- ✅ **Fluent Interface**: Builder pattern for easy component creation
- ✅ **JSON Serialization**: Automatic conversion to/from Discord API format
- ✅ **Validation**: Built-in validation for all components
- ✅ **Modular Design**: Each component type in its own header
- ✅ **Component V2 Support**: Latest Discord component features
- ✅ **Performance Optimized**: Efficient memory usage and caching
- ✅ **Thread Safe**: Safe for concurrent read operations

## 📋 Requirements

- C++17 or higher
- nlohmann/json library
- Standard library dependencies

## 🛠️ Installation

1. Include the components headers in your project:
```cpp
#include "discord/components.h"
```

2. Link against nlohmann/json and compile with C++17:
```bash
g++ -std=c++17 your_code.cpp -ljson
```

## 🎯 Quick Start

### Basic Usage

```cpp
#include "discord/components.h"

using namespace discord::components;

// Create a simple button
auto button = Button::primary("Click me!", "my_button");

// Create a select menu
std::vector<SelectOption> options = {
    SelectOption("Option 1", "opt1"),
    SelectOption("Option 2", "opt2")
};
auto select = StringSelect::create("my_select", options);

// Create an action row
auto action_row = ActionRow::with_buttons({button});

// Convert to JSON for Discord API
nlohmann::json json = action_row->to_json();
```

### Builder Pattern

```cpp
auto components = ComponentBuilder::create()
    .new_row()
    .primary_button("Save", "save")
    .danger_button("Delete", "delete")
    .finish_current()
    .new_section("Options")
    .string_select("options", options, "Choose options")
    .finish_current()
    .build_all();
```

## 📚 Component Types

### Buttons

```cpp
// Different button styles
auto primary = Button::primary("Primary", "primary_btn");
auto secondary = Button::secondary("Secondary", "secondary_btn");
auto success = Button::success("Success", "success_btn");
auto danger = Button::danger("Danger", "danger_btn");
auto link = Button::link("Open Link", "https://example.com");

// With emoji
auto button_with_emoji = Button::primary("React", "react_btn", "😀");
```

### Select Menus

```cpp
// String select
auto string_select = StringSelect::create("my_select", options, "Choose an option");

// User select
auto user_select = UserSelect::create("user_select", "Select users", 1, 5);

// Role select
auto role_select = RoleSelect::create("role_select", "Select roles");

// Channel select
auto channel_select = ChannelSelect::create("channel_select", {"0", "2"}, "Select channels");

// Mentionable select
auto mentionable_select = MentionableSelect::create("mention_select", "Select users or roles");
```

### Text Inputs

```cpp
// Short text input
auto short_input = TextInput::short_text("name", "Name", "", "Enter name", true);

// Paragraph text input
auto paragraph_input = TextInput::paragraph("bio", "Bio", "", "Tell us about yourself");
```

### Containers

```cpp
// Action row with buttons
auto action_row = ActionRow::with_buttons({
    Button::primary("Yes", "yes"),
    Button::danger("No", "no")
});

// Section with text and accessory
auto section = Section::with_button("Confirm action?", 
                                     Button::success("Confirm", "confirm"));

// Container with mixed components
auto container = Container::mixed({
    std::make_shared<TextDisplay>("Some text"),
    std::make_shared<Button>("Click", "click")
});
```

## 🔧 Advanced Features

### Validation

```cpp
ValidationResult result = ComponentUtils::validate_component_list(components);
if (!result.valid) {
    for (const auto& error : result.errors) {
        std::cout << "Error: " << error << std::endl;
    }
}
```

### JSON Serialization

```cpp
// Serialize to JSON
nlohmann::json json = ComponentJson::serialize_components(components);

// Deserialize from JSON
auto components = ComponentJson::deserialize_components(json);

// Save to file
ComponentJson::save_components_to_file(components, "layout.json");

// Load from file
auto loaded = ComponentJson::load_components_from_file("layout.json");
```

### Auto-Layout

```cpp
// Automatically arrange buttons into rows
std::vector<std::shared_ptr<Button>> buttons = {/* many buttons */};
auto layout = ComponentUtils::auto_layout_buttons(buttons, 3); // 3 per row
```

### Component Factory

```cpp
auto button = ComponentFactory::create_button("Click", "click", "primary");
auto select = ComponentFactory::create_select_menu("select", options);
auto text_input = ComponentFactory::create_text_input("input", "Label");
```

## 🆕 Component V2 Features

The system supports Discord's latest Components V2 features:

- **Sections**: Text content with optional accessory components
- **Containers**: Flexible layout containers
- **Content Display**: Rich content components (thumbnails, media, files)
- **Enhanced Layout**: Better organization and responsive design

```cpp
// Component V2 example
auto section = Section::create("Welcome to the server!", 
                               Button::primary("Get Started", "start"));

auto container = Container::create({
    std::make_shared<Section>("Section 1"),
    std::make_shared<Section>("Section 2"),
    std::make_shared<Thumbnail>("https://example.com/image.png")
});
```

## 🔗 Integration with Discord.cpp

The components system integrates seamlessly with the main Discord.cpp client:

```cpp
DiscordClient client("your_token");

// Handle button interactions
client.on_button_click([](const std::string& custom_id, const nlohmann::json& interaction) {
    if (custom_id == "my_button") {
        // Handle button click
        client.send_interaction_response(interaction, "Button clicked!");
    }
});

// Send message with components
auto components = ActionRow::with_buttons({
    Button::primary("Click me", "my_button")
});

client.send_message(channel_id, "Check out these buttons:", components);
```

## 📖 Best Practices

1. **Use Custom IDs**: Always provide meaningful custom IDs for interactive components
2. **Validate Components**: Use built-in validation before sending to Discord
3. **Handle Errors**: Always check validation results and handle errors gracefully
4. **Use Builders**: For complex layouts, use the builder pattern for cleaner code
5. **Component Limits**: Respect Discord's component limits (5 components per action row, etc.)

## ⚠️ Error Handling

The components system provides comprehensive error handling:

```cpp
try {
    auto button = Button::primary("", ""); // Empty label - will throw
} catch (const std::invalid_argument& e) {
    std::cerr << "Invalid button: " << e.what() << std::endl;
}

// Validation provides detailed error messages
ValidationResult result = button->validate();
if (!result.valid) {
    for (const auto& error : result.errors) {
        std::cerr << "Validation error: " << error << std::endl;
    }
}
```

## ⚡ Performance Considerations

- Use `ComponentCache` for frequently used components
- Batch component creation when possible
- Use `ComponentUtils::optimize_component_tree()` for large component trees
- Consider using JSON serialization for persistent component layouts

## 🧵 Thread Safety

All component classes are thread-safe for read operations. For write operations, use appropriate synchronization or create copies for different threads.

## 📁 Examples

See the `examples/` directory for comprehensive examples:

- `simple_test.cpp`: Basic component functionality test
- `components_example.cpp`: Comprehensive component usage examples
- `builder_example.cpp`: Advanced builder patterns
- `validation_example.cpp`: Component validation examples
- `serialization_example.cpp`: JSON serialization examples

## 📚 API Reference

For detailed API documentation, see the header files in `include/discord/components/`. Each component type is fully documented with examples and usage notes.

## 🏗️ Architecture

The components system follows a modular architecture:

```
include/discord/components/
├── component_base.h      # Base interfaces and types
├── button.h              # Button component
├── select_menu.h         # Select menu components
├── text_input.h          # Text input component
├── action_row.h          # Action row container
├── section.h             # Section component
├── container.h           # Container component
├── content_display.h     # Content display components
├── embed_components.h    # Embed-specific components
├── component_builder.h   # Builder pattern
├── component_json.h       # JSON serialization
├── component_utils.h     # Utility functions
└── components.h          # Barrel header
```

## 🤝 Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Submit a pull request

## 📄 License

This project is licensed under the MIT License - see the LICENSE file for details.

## 🆘 Support

- Create an issue for bug reports
- Check the documentation for common questions
- Look at the examples for usage patterns

## 🗺️ Roadmap

- [ ] Add more component validation rules
- [ ] Implement component theming
- [ ] Add component animation support
- [ ] Create visual component builder
- [ ] Add component testing utilities
- [ ] Implement component caching strategies

---

**Discord.cpp Components System** - Making Discord UI components simple and type-safe in C++!