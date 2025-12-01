#include <discord/utils/logger.h>
#include <discord/core/exceptions.h>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

namespace discord {

// Global logger instance
std::shared_ptr<Logger> g_logger = nullptr;

Logger::Logger(
    Level level,
    Output output,
    const std::string& log_file,
    size_t max_buffer_size
) : current_level_(level)
  , output_(output)
  , log_file_(log_file)
  , max_buffer_size_(max_buffer_size)
  , colors_enabled_(true) {
    
    if (output_ == Output::File || output_ == Output::Both) {
        if (log_file_.empty()) {
            throw DiscordException("Log file path required when output includes File");
        }
        initialize_file();
    }
}

Logger::~Logger() {
    flush();
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
}

void Logger::log(Level level, const std::string& message) {
    if (level < current_level_) {
        return;
    }

    LogEntry entry{
        level,
        message,
        std::chrono::system_clock::now(),
        get_thread_id()
    };

    std::lock_guard<std::mutex> lock(mutex_);

    // Add to buffer
    if (buffer_.size() >= max_buffer_size_) {
        buffer_.erase(buffer_.begin());
    }
    buffer_.push_back(entry);

    // Write to outputs
    if (output_ == Output::Console || output_ == Output::Both) {
        write_to_console(entry);
    }
    
    if (output_ == Output::File || output_ == Output::Both) {
        write_to_file(entry);
    }
}

void Logger::set_level(Level level) {
    std::lock_guard<std::mutex> lock(mutex_);
    current_level_ = level;
}

Logger::Level Logger::get_level() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return current_level_;
}

void Logger::set_output(Output output, const std::string& log_file) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Close existing file stream if open
    if (file_stream_.is_open()) {
        file_stream_.close();
    }
    
    output_ = output;
    if (!log_file.empty()) {
        log_file_ = log_file;
    }
    
    if (output_ == Output::File || output_ == Output::Both) {
        if (log_file_.empty()) {
            throw DiscordException("Log file path required when output includes File");
        }
        initialize_file();
    }
}

void Logger::set_colors_enabled(bool enabled) {
    std::lock_guard<std::mutex> lock(mutex_);
    colors_enabled_ = enabled;
}

std::vector<Logger::LogEntry> Logger::get_buffer() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return buffer_;
}

void Logger::clear_buffer() {
    std::lock_guard<std::mutex> lock(mutex_);
    buffer_.clear();
}

void Logger::flush() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (file_stream_.is_open()) {
        file_stream_.flush();
    }
}

void Logger::trace(const std::string& message) {
    log(Level::Trace, message);
}

void Logger::debug(const std::string& message) {
    log(Level::Debug, message);
}

void Logger::info(const std::string& message) {
    log(Level::Info, message);
}

void Logger::warn(const std::string& message) {
    log(Level::Warning, message);
}

void Logger::error(const std::string& message) {
    log(Level::Error, message);
}

void Logger::critical(const std::string& message) {
    log(Level::Critical, message);
}

std::string Logger::level_to_string(Level level) const {
    switch (level) {
        case Level::Trace: return "TRACE";
        case Level::Debug: return "DEBUG";
        case Level::Info: return "INFO";
        case Level::Warning: return "WARN";
        case Level::Error: return "ERROR";
        case Level::Critical: return "CRITICAL";
        default: return "UNKNOWN";
    }
}

std::string Logger::format_timestamp(std::chrono::system_clock::time_point timestamp) const {
    auto time_t = std::chrono::system_clock::to_time_t(timestamp);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        timestamp.time_since_epoch()
    ) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string Logger::get_thread_id() const {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

std::string Logger::apply_colors(Level level, const std::string& message) const {
    if (!colors_enabled_) {
        return message;
    }
    
    const char* color_code = "";
    const char* reset_code = "\033[0m";
    
    switch (level) {
        case Level::Trace: color_code = "\033[37m"; break;    // White
        case Level::Debug: color_code = "\033[36m"; break;    // Cyan
        case Level::Info: color_code = "\033[32m"; break;     // Green
        case Level::Warning: color_code = "\033[33m"; break;  // Yellow
        case Level::Error: color_code = "\033[31m"; break;   // Red
        case Level::Critical: color_code = "\033[35m"; break; // Magenta
    }
    
    return std::string(color_code) + message + reset_code;
}

void Logger::write_to_console(const LogEntry& entry) const {
    std::string level_str = level_to_string(entry.level);
    std::string timestamp_str = format_timestamp(entry.timestamp);
    
    std::stringstream ss;
    ss << "[" << timestamp_str << "] "
       << "[" << level_str << "] "
       << "[Thread:" << entry.thread_id << "] "
       << entry.message;
    
    std::string output = apply_colors(entry.level, ss.str());
    std::cout << output << std::endl;
}

void Logger::write_to_file(const LogEntry& entry) {
    if (!file_stream_.is_open()) {
        return;
    }
    
    std::string level_str = level_to_string(entry.level);
    std::string timestamp_str = format_timestamp(entry.timestamp);
    
    file_stream_ << "[" << timestamp_str << "] "
                 << "[" << level_str << "] "
                 << "[Thread:" << entry.thread_id << "] "
                 << entry.message << std::endl;
}

void Logger::initialize_file() {
    file_stream_.open(log_file_, std::ios::app);
    if (!file_stream_.is_open()) {
        throw DiscordException("Failed to open log file: " + log_file_);
    }
}

void initialize_logger(
    Logger::Level level,
    Logger::Output output,
    const std::string& log_file
) {
    g_logger = std::make_shared<Logger>(level, output, log_file);
}

std::shared_ptr<Logger> get_logger() {
    if (!g_logger) {
        // Initialize with default settings if not already initialized
        g_logger = std::make_shared<Logger>();
    }
    return g_logger;
}

} // namespace discord