#pragma once

#include <expected>
#include <format>
#include <iosfwd>
#include <string>
#include <string_view>
#include <utility>

namespace logcoe
{
    enum class log_level
    {
        debug,
        info,
        warning,
        error,
        none
    };

    enum class error_reason
    {
        file_open_failure,
        invalid_time_format,
        not_initialized
    };

    void initialize(log_level level = log_level::debug,
                    const std::string &default_source = "",
                    bool enable_console = true,
                    bool enable_file = false,
                    const std::string &filename = "logcoe.log");
    void shutdown();

    void set_log_level(log_level level);
    void set_console_output(std::ostream &stream);
    [[nodiscard]] std::expected<void, error_reason> set_file_output(const std::string &filename);
    void disable_console_output();
    void disable_file_output();
    [[nodiscard]] std::expected<void, error_reason> set_time_format(const std::string &format);

    [[nodiscard]] bool is_initialized();
    [[nodiscard]] log_level get_log_level();

    void debug(std::string_view message, std::string_view source = "", bool flush_ = true);
    void info(std::string_view message, std::string_view source = "", bool flush_ = true);
    void warning(std::string_view message, std::string_view source = "", bool flush_ = true);
    void error(std::string_view message, std::string_view source = "", bool flush_ = true);
    void flush();

    template <typename... args_t> void debugf(std::format_string<args_t...> fmt, args_t &&...args)
    {
        if (get_log_level() > log_level::debug) return;
        debug(std::format(fmt, std::forward<args_t>(args)...));
    }

    template <typename... args_t> void infof(std::format_string<args_t...> fmt, args_t &&...args)
    {
        if (get_log_level() > log_level::info) return;
        info(std::format(fmt, std::forward<args_t>(args)...));
    }

    template <typename... args_t> void warningf(std::format_string<args_t...> fmt, args_t &&...args)
    {
        if (get_log_level() > log_level::warning) return;
        warning(std::format(fmt, std::forward<args_t>(args)...));
    }

    template <typename... args_t> void errorf(std::format_string<args_t...> fmt, args_t &&...args)
    {
        if (get_log_level() > log_level::error) return;
        error(std::format(fmt, std::forward<args_t>(args)...));
    }

} // namespace logcoe
