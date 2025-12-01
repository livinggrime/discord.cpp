#pragma once

// Build configuration and version information
#define DISCORD_CPP_VERSION_MAJOR 1
#define DISCORD_CPP_VERSION_MINOR 0
#define DISCORD_CPP_VERSION_PATCH 0

#define DISCORD_CPP_VERSION \
    ((DISCORD_CPP_VERSION_MAJOR * 10000) + \
     (DISCORD_CPP_VERSION_MINOR * 100) + \
     DISCORD_CPP_VERSION_PATCH)

// Feature flags
#define DISCORD_CPP_ENABLE_CACHE 1
#define DISCORD_CPP_ENABLE_RATE_LIMITER 1
#define DISCORD_CPP_ENABLE_LOGGING 1
#define DISCORD_CPP_ENABLE_THREADING 1

// Platform detection
#if defined(_WIN32)
    #define DISCORD_CPP_PLATFORM_WINDOWS 1
#elif defined(__APPLE__)
    #define DISCORD_CPP_PLATFORM_MACOS 1
#elif defined(__linux__)
    #define DISCORD_CPP_PLATFORM_LINUX 1
#else
    #define DISCORD_CPP_PLATFORM_UNKNOWN 1
#endif

// Compiler detection and feature support
#if __cplusplus >= 202002L
    #define DISCORD_CPP_CXX20 1
#else
    #define DISCORD_CPP_CXX20 0
#endif
