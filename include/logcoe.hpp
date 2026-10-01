#pragma once

#include <string>
#include <expected>

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
        invalid_time_format
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

    bool is_initialized();
    log_level get_log_level();

    void debug(const std::string &message, const std::string &source = "", bool flush_ = true);
    void info(const std::string &message, const std::string &source = "", bool flush_ = true);
    void warning(const std::string &message, const std::string &source = "", bool flush_ = true);
    void error(const std::string &message, const std::string &source = "", bool flush_ = true);
    void flush();

} // namespace logcoe