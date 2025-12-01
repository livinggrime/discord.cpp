#pragma once

#include "../core/interfaces.h"
#include <memory>
#include <string>
#include <fstream>
#include <mutex>
#include <vector>
#include <sstream>
#include <thread>
#include <iomanip>
#include <iostream>

namespace discord {

/**
 * @brief Logger implementation for Discord.cpp
 * 
 * Provides thread-safe logging with multiple output destinations
 * and configurable log levels. Implements the ILogger interface.
 * 
 * @todo TODO.md: Complete missing implementations: Logger
 */
class Logger : public ILogger {
public:
    /**
     * @brief Log output destination
     */
    enum class Output {
        Console,
        File,
        Both
    };

    /**
     * @brief Log entry structure
     */
    struct LogEntry {
        Level level;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        std::string thread_id;
    };

private:
    Level current_level_;
    Output output_;
    std::string log_file_;
    std::ofstream file_stream_;
    mutable std::mutex mutex_;
    std::vector<LogEntry> buffer_;
    size_t max_buffer_size_;
    bool colors_enabled_;

    /**
     * @brief Format log level to string
     * @param level Log level to format
     * @return Formatted string representation
     */
    std::string level_to_string(Level level) const;

    /**
     * @brief Format timestamp to string
     * @param timestamp Time point to format
     * @return Formatted timestamp string
     */
    std::string format_timestamp(std::chrono::system_clock::time_point timestamp) const;

    /**
     * @brief Get current thread ID as string
     * @return Thread ID string
     */
    std::string get_thread_id() const;

    /**
     * @brief Apply console colors to log message
     * @param level Log level for color determination
     * @param message Message to colorize
     * @return Colorized message
     */
    std::string apply_colors(Level level, const std::string& message) const;

    /**
     * @brief Write log entry to console
     * @param entry Log entry to write
     */
    void write_to_console(const LogEntry& entry) const;

    /**
     * @brief Write log entry to file
     * @param entry Log entry to write
     */
    void write_to_file(const LogEntry& entry);

    /**
     * @brief Initialize file output
     */
    void initialize_file();

public:
    /**
     * @brief Construct Logger
     * @param level Minimum log level (default: Info)
     * @param output Output destination (default: Console)
     * @param log_file Path to log file (required if output includes File)
     * @param max_buffer_size Maximum number of entries to buffer (default: 1000)
     */
    explicit Logger(
        Level level = Level::Info,
        Output output = Output::Console,
        const std::string& log_file = "",
        size_t max_buffer_size = 1000
    );

    /**
     * @brief Destructor - closes file stream if open
     */
    ~Logger() override;

    /**
     * @brief Log a message at specified level
     * @param level Log level
     * @param message Message to log
     */
    void log(Level level, const std::string& message) override;

    /**
     * @brief Set minimum log level
     * @param level New minimum log level
     */
    void set_level(Level level) override;

    /**
     * @brief Get current minimum log level
     * @return Current log level
     */
    Level get_level() const override;

    /**
     * @brief Set output destination
     * @param output New output destination
     * @param log_file Path to log file (required if output includes File)
     */
    void set_output(Output output, const std::string& log_file = "");

    /**
     * @brief Enable/disable console colors
     * @param enabled Whether to enable colors
     */
    void set_colors_enabled(bool enabled);

    /**
     * @brief Get buffered log entries
     * @return Vector of buffered log entries
     */
    std::vector<LogEntry> get_buffer() const;

    /**
     * @brief Clear log buffer
     */
    void clear_buffer();

    /**
     * @brief Flush all pending logs
     */
    void flush();

    // Convenience methods for different log levels
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);
    void critical(const std::string& message);
};

/**
 * @brief Global logger instance
 */
extern std::shared_ptr<Logger> g_logger;

/**
 * @brief Initialize global logger
 * @param level Log level
 * @param output Output destination
 * @param log_file Log file path
 */
void initialize_logger(
    Logger::Level level = Logger::Level::Info,
    Logger::Output output = Logger::Output::Console,
    const std::string& log_file = ""
);

/**
 * @brief Get global logger instance
 * @return Shared pointer to global logger
 */
std::shared_ptr<Logger> get_logger();

/**
 * @brief Convenience macros for logging
 */
#define LOG_TRACE(msg) discord::get_logger()->trace(msg)
#define LOG_DEBUG(msg) discord::get_logger()->debug(msg)
#define LOG_INFO(msg) discord::get_logger()->info(msg)
#define LOG_WARN(msg) discord::get_logger()->warn(msg)
#define LOG_ERROR(msg) discord::get_logger()->error(msg)
#define LOG_CRITICAL(msg) discord::get_logger()->critical(msg)

} // namespace discord