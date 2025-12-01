cmake_minimum_required(VERSION 3.20)
project(discordcpp 
    VERSION 1.0.0 
    LANGUAGES CXX
    DESCRIPTION "Modern C++ Discord API wrapper"
    HOMEPAGE_URL "https://github.com/yourusername/discord.cpp"
)

# ========== PROJECT CONFIGURATION ==========
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# ========== BUILD OPTIONS ==========
option(BUILD_SHARED_LIBS "Build discord_cpp as shared library" OFF)
option(BUILD_EXAMPLES "Build example programs" ON)
option(BUILD_TESTS "Build tests" ON)
option(ENABLE_CODE_COVERAGE "Enable code coverage analysis" OFF)

# ========== MAIN LIBRARY ==========
add_subdirectory(src)

# ========== EXAMPLES ==========
if(BUILD_EXAMPLES)
    add_executable(discord_py_like_bot examples/discord_py_like_working_bot.cpp)
    target_link_libraries(discord_py_like_bot PRIVATE discord_cpp)
    target_include_directories(discord_py_like_bot PRIVATE ${CMAKE_SOURCE_DIR}/include)
endif()

# ========== TESTING ==========
if(BUILD_TESTS)
    include(CTest)
    enable_testing()
    # Add test subdirectory if it exists
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests)
        add_subdirectory(tests)
    endif()
endif()

# ========== EXPORT TARGETS ==========
# Create package config files for consumers to use find_package()
include(CMakePackageConfigHelpers)

# Create targets file
export(TARGETS discord_cpp
    FILE "${CMAKE_BINARY_DIR}/discord_cppTargets.cmake"
)

# Create config file
configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/discord_cppConfig.cmake.in"
    "${CMAKE_BINARY_DIR}/discord_cppConfig.cmake"
    INSTALL_DESTINATION lib/cmake/discord_cpp
)

# ========== PRINT CONFIGURATION ==========
message(STATUS "==================================")
message(STATUS "discord.cpp v${PROJECT_VERSION}")
message(STATUS "==================================")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build Shared Libs: ${BUILD_SHARED_LIBS}")
message(STATUS "Build Examples: ${BUILD_EXAMPLES}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "==================================")
