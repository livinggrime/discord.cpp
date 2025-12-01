# Development Guide

## Quick Start for Developers

### Prerequisites

- CMake 3.20+
- C++23 compatible compiler (GCC 13+, Clang 17+, MSVC 2022+)
- Required libraries:
  - nlohmann_json
  - OpenSSL
  - libcurl
  - websocketpp (optional, for WebSocket support)

### Building the Library

```bash
# Clone the repository
git clone https://github.com/yourusername/discord.cpp.git
cd discord.cpp

# Create build directory
mkdir build && cd build

# Configure CMake
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_EXAMPLES=ON \
  -DBUILD_TESTS=ON

# Build the project
cmake --build . -j$(nproc)

# Install headers and library
cmake --install . --prefix /usr/local
```

### Development Workflow

#### Adding a New Feature to an Existing Module

1. **Identify the module** - Find which module your feature belongs to
2. **Update the interface** if needed: `include/discord/module/header.h`
3. **Implement the feature** in: `src/module/implementation.cpp`
4. **Update CMakeLists** if adding new files
5. **Test your changes** with the test suite or examples
6. **Update documentation** if the feature is user-facing

#### Creating a New Module

Example: Creating a `discord::voice` module

1. **Create headers**:
   ```
   include/discord/voice/
   ├── voice_client.h
   ├── voice_state.h
   └── pcm_source.h
   ```

2. **Create implementations**:
   ```
   src/voice/
   ├── voice_client.cpp
   ├── voice_state.cpp
   └── pcm_source.cpp
   ```

3. **Update src/CMakeLists.txt**:
   ```cmake
   # ========== VOICE MODULE ==========
   voice/voice_client.cpp
   voice/voice_state.cpp
   voice/pcm_source.cpp
   ```

4. **Create module header** `include/discord/voice.h`:
   ```cpp
   #pragma once
   #include "config.h"
   #include "voice/voice_client.h"
   #include "voice/voice_state.h"
   #include "voice/pcm_source.h"
   
   namespace discord::voice {
       using discord::VoiceClient;
       using discord::VoiceState;
       using discord::PCMSource;
   }
   ```

5. **Update main header** `include/discord.h`:
   ```cpp
   #include "voice.h"
   ```

6. **Document** in the module header file

### File Organization

Each module should follow this structure:

```
module/
├── header1.h         # Public interface
├── header2.h         # Public interface
├── internal/
│   ├── helper.h      # Internal headers (not exposed)
│   └── detail.h
└── 
```

### Naming Conventions

- **Classes**: PascalCase (`DiscordClient`, `HTTPClient`)
- **Functions**: snake_case (`send_message()`, `connect()`)
- **Variables**: snake_case (`channel_id`, `message_count`)
- **Constants**: UPPER_SNAKE_CASE (`API_VERSION`, `DEFAULT_TIMEOUT`)
- **Namespaces**: lowercase (`discord::api`, `discord::gateway`)
- **Files**: lowercase with underscores (`http_client.h`, `websocket_client.cpp`)

### Code Style

- Use `#pragma once` for header guards
- 4-space indentation (or tabs if preferred)
- Include order:
  1. Standard library headers
  2. Third-party headers
  3. Local project headers
- Use forward declarations to reduce dependencies
- Document public APIs with comments

### Testing

```bash
# Run tests
ctest --build-config Release -VV

# Build and run a specific example
./bin/discord_py_like_bot
```

### Common Development Tasks

#### Fixing a Bug

1. Write a test case that reproduces the bug
2. Locate the affected module and file
3. Fix the bug while keeping tests passing
4. Update the example or documentation if needed

#### Optimizing Performance

1. Profile the code to identify bottlenecks
2. Use appropriate data structures (cache lookup patterns)
3. Consider async/await patterns for blocking operations
4. Use thread pools for parallel tasks

#### Adding Error Handling

- Use exception hierarchy defined in `core/exceptions.h`
- Provide meaningful error messages
- Document expected exceptions in function comments

### Debugging

```bash
# Build with debug symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Use gdb or lldb
gdb ./bin/discord_py_like_bot
```

### Documentation

- Document public APIs using comment blocks
- Update MODULAR_ARCHITECTURE.md for structural changes
- Add examples in the examples/ directory
- Include error cases in documentation

### Version Management

Follow semantic versioning (MAJOR.MINOR.PATCH):
- Update version in `include/discord/config.h`
- Update version in `CMakeLists.txt`
- Create git tags for releases

### Integration with IDE

#### VS Code
- Install C/C++ Extension
- Install CMake Tools Extension
- Open workspace, configure CMake

#### CLion
- Open as CMake project
- Should auto-detect configuration
- Enable C++23 in settings if needed

#### Visual Studio
- Use "Open as Folder" or generate Visual Studio solution
- CMake integration is automatic in VS 2019+

### Performance Tips

1. Use `Result<T>` for error handling instead of exceptions in hot paths
2. Leverage thread pool for background tasks
3. Cache frequently accessed data
4. Use move semantics for large objects
5. Profile with `perf` or `Instruments` before optimizing

### Security Considerations

- Token validation and protection
- Rate limit compliance
- Input validation for user data
- Secure storage of sensitive information
- Use TLS for all network connections

### Troubleshooting

**Issue**: Build fails with C++23 requirement
- Solution: Upgrade compiler or update CMakeLists.txt to use C++20

**Issue**: Can't find nlohmann_json
- Solution: Install via package manager or provide path to CMake

**Issue**: WebSocket errors
- Solution: Ensure websocketpp is installed and include path is set

**Issue**: SSL/TLS certificate errors
- Solution: Ensure OpenSSL is properly installed and certificates are valid
