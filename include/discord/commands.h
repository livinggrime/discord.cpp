#pragma once

/**
 * @file commands.h
 * @brief Commands module for command handling and management
 */

#include "config.h"
#include "commands/command_manager.h"

namespace discord::commands {
    // Re-export commonly used types
    using discord::CommandManager;
} // namespace discord::commands
