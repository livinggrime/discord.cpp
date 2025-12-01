#pragma once

#include "../utils/types.h"
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <future>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <nlohmann/json.hpp>

namespace discord {

// Forward declarations
class DiscordClient;
class ILogger;

// Command context for execution
struct CommandContext {
    User author;
    Channel channel;
    std::optional<Guild> guild;
    std::string content;
    std::vector<std::string> args;
    std::string message_id;
    DiscordClient* client;
    
    CommandContext(DiscordClient* client) : client(client) {}
};

// Command option types for slash commands
enum class CommandOptionType {
    SUB_COMMAND = 1,
    SUB_COMMAND_GROUP = 2,
    STRING = 3,
    INTEGER = 4,
    BOOLEAN = 5,
    USER = 6,
    CHANNEL = 7,
    ROLE = 8,
    MENTIONABLE = 9,
    NUMBER = 10,
    ATTACHMENT = 11
};

// Command option structure
struct CommandOption {
    CommandOptionType type;
    std::string name;
    std::string description;
    bool required;
    std::vector<CommandOption> choices;
    std::vector<CommandOption> options;
    
    CommandOption(CommandOptionType t, const std::string& n, const std::string& desc, bool req = false)
        : type(t), name(n), description(desc), required(req) {}
};

// Command interface
class ICommand {
public:
    virtual ~ICommand() = default;
    virtual void execute(const CommandContext& ctx) = 0;
    virtual std::string get_name() const = 0;
    virtual std::string get_description() const = 0;
    virtual std::vector<std::string> get_aliases() const = 0;
    virtual std::vector<CommandOption> get_options() const = 0;
    virtual int get_permissions() const { return 0; }
    virtual bool is_nsfw() const { return false; }
    virtual bool can_use_in_dm() const { return true; }
};

// Middleware interface
class IMiddleware {
public:
    virtual ~IMiddleware() = default;
    virtual std::future<bool> before(const CommandContext& ctx) = 0;
    virtual std::future<void> after(const CommandContext& ctx, bool success) = 0;
    virtual int get_priority() const { return 0; }
};

// Command manager
class CommandManager {
private:
    std::unordered_map<std::string, std::unique_ptr<ICommand>> commands_;
    std::vector<std::unique_ptr<IMiddleware>> middleware_;
    std::string prefix_;
    DiscordClient* client_;
    
public:
    explicit CommandManager(DiscordClient* client, const std::string& prefix = "!");
    ~CommandManager() = default;
    
    void register_command(std::unique_ptr<ICommand> command);
    void unregister_command(const std::string& name);
    void add_middleware(std::unique_ptr<IMiddleware> middleware);
    
    void handle_message(const Message& message);
    void handle_interaction(const nlohmann::json& interaction);
    
    ICommand* get_command(const std::string& name) const;
    std::vector<std::string> get_command_names() const;
    
    void set_prefix(const std::string& prefix);
    const std::string& get_prefix() const;
    
    // Slash command registration
    std::future<void> register_slash_commands(const std::string& guild_id = "");
    nlohmann::json create_slash_command_json(const ICommand& command) const;
    
private:
    std::vector<std::string> parse_args(const std::string& content, const std::string& prefix) const;
    std::string find_command(const std::string& name) const;
    std::future<bool> execute_middleware_before(const CommandContext& ctx);
    std::future<void> execute_middleware_after(const CommandContext& ctx, bool success);
};

// Built-in middleware classes
class CooldownMiddleware : public IMiddleware {
private:
    std::unordered_map<std::string, std::chrono::system_clock::time_point> cooldowns_;
    std::chrono::seconds cooldown_duration_;
    mutable std::mutex mutex_;
    
public:
    explicit CooldownMiddleware(std::chrono::seconds duration);
    std::future<bool> before(const CommandContext& ctx) override;
    std::future<void> after(const CommandContext& ctx, bool success) override;
};

class PermissionMiddleware : public IMiddleware {
public:
    std::future<bool> before(const CommandContext& ctx) override;
    std::future<void> after(const CommandContext& ctx, bool success) override;
};

class LoggingMiddleware : public IMiddleware {
private:
    ILogger* logger_;
    
public:
    explicit LoggingMiddleware(ILogger* logger);
    std::future<bool> before(const CommandContext& ctx) override;
    std::future<void> after(const CommandContext& ctx, bool success) override;
};

} // namespace discord