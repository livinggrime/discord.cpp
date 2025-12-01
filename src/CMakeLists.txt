# Discord.cpp Library Build Configuration
# ==========================================

# Define the library target
add_library(discord_cpp

    # ========== CORE MODULE ==========
    # Core client and exception handling
    core/client.cpp
    core/exceptions.cpp

    # ========== API MODULE ==========
    # HTTP client and REST endpoints
    api/http_client.cpp
    api/rest_endpoints.cpp
    api/rate_limiter.cpp

    # ========== GATEWAY MODULE ==========
    # WebSocket connection and gateway events
    gateway/websocket_client.cpp
    gateway/reconnection.cpp
    gateway/gateway_events.cpp
    gateway/shard_manager.cpp

    # ========== EVENTS MODULE ==========
    # Event handling and dispatch
    events/event_dispatcher.cpp
    events/event_handlers.cpp
    events/middleware.cpp

    # ========== CACHE MODULE ==========
    # Caching layer implementations
    cache/cache_manager.cpp
    cache/memory_cache.cpp
    cache/redis_cache.cpp

    # ========== UTILITIES MODULE ==========
    # Common utilities and helpers
    utils/auth.cpp
    utils/config_manager.cpp
    utils/types.cpp
    utils/thread_pool.cpp
    utils/logger.cpp
    utils/embed_builder.cpp
)

# Configure include directories
target_include_directories(discord_cpp
    PUBLIC
        # Public includes for library consumers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        # Private includes for implementation
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# ========== DEPENDENCIES ==========
# Find required external packages
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL REQUIRED)

# Optional WebSocket support (websocketpp)
if(NOT DEFINED WEBSOCKETPP_INCLUDE_DIRS)
    # Try to find websocketpp
    find_path(WEBSOCKETPP_INCLUDE_DIRS websocketpp/client.hpp)
    if(WEBSOCKETPP_INCLUDE_DIRS)
        message(STATUS "Found websocketpp: ${WEBSOCKETPP_INCLUDE_DIRS}")
    else()
        message(WARNING "websocketpp not found - WebSocket support may be limited")
    endif()
endif()

# ========== LINK LIBRARIES ==========
# Link required libraries
target_link_libraries(discord_cpp
    PUBLIC
        nlohmann_json::nlohmann_json
        OpenSSL::SSL
        OpenSSL::Crypto
    PRIVATE
        ${CURL_LIBRARIES}
)

# Add include directories for WebSocket
if(WEBSOCKETPP_INCLUDE_DIRS)
    target_include_directories(discord_cpp PRIVATE ${WEBSOCKETPP_INCLUDE_DIRS})
    target_compile_definitions(discord_cpp PRIVATE DISCORD_CPP_USE_WEBSOCKETPP)
endif()

# ========== COMPILATION FLAGS ==========
# Set C++ standard requirements
set_target_properties(discord_cpp PROPERTIES
    CXX_STANDARD 23
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

# Enable position-independent code for shared libraries
set_target_properties(discord_cpp PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

# Add compiler-specific optimizations
if(MSVC)
    target_compile_options(discord_cpp PRIVATE /W4 /WX)
else()
    target_compile_options(discord_cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ========== INSTALLATION RULES ==========
# Install library
install(TARGETS discord_cpp
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

# Install headers
install(DIRECTORY ../include/discord
    DESTINATION include
)