#include <logcoe.hpp>
#include <chrono>
#include <cstddef>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string_view>
#include <system_error>
#include <utility>

using logcoe::log_level;

#ifndef NDEBUG
namespace
{
    unsigned int g_init_counter = 0;
    log_level g_log_level = log_level::info;
    std::string g_default_source = "";
    std::mutex g_mutex;
    std::string g_filename = "logcoe.log";
    std::ofstream g_file_stream;
    std::ostream *g_console_stream = &std::cout;
    bool g_use_file = false;
    bool g_use_console = true;
    std::string g_time_format = "%d/%m/%Y__%H:%M:%S";

    namespace internal
    {

        std::string format_time(const std::string &format)
        {
            auto now = std::chrono::system_clock::now();
            std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);

            std::tm tm_now;
    #ifdef _WIN32
            localtime_s(&tm_now, &time_t_now);
    #else
            localtime_r(&time_t_now, &tm_now);
    #endif

            char buffer[256];
            std::strftime(buffer, sizeof(buffer), format.c_str(), &tm_now);

            return std::string(buffer);
        }

        std::string get_current_timestamp()
        {
            if (g_init_counter == 0) return "";

            return format_time(g_time_format);
        }

        std::string get_default_filename() { return "logcoe_" + format_time("%Y-%m-%d_%H-%M-%S") + ".log"; }

        constexpr std::string_view get_log_level_as_string(log_level level)
        {
            switch (level)
            {
                case log_level::debug:
                    return "DEBUG";
                case log_level::info:
                    return "INFO";
                case log_level::warning:
                    return "WARNING";
                case log_level::error:
                    return "ERROR";
                case log_level::none:
                    return "NONE";
            }
            return "NONE";
        }

        void write_to_outputs(const std::string &formatted_message,
                              log_level level = log_level::info,
                              bool flush_ = true)
        {
            if (g_init_counter == 0 || std::to_underlying(level) < std::to_underlying(g_log_level)) return;

            if (g_use_console && g_console_stream)
            {
                *g_console_stream << formatted_message << '\n';
                if (flush_) g_console_stream->flush();
            }

            if (g_use_file)
            {
                g_file_stream << formatted_message << '\n';
                if (flush_) g_file_stream.flush();
            }
        }

        void flush_streams()
        {
            if (g_use_console && g_console_stream) g_console_stream->flush();
            if (g_use_file && g_file_stream.is_open()) g_file_stream.flush();
        }

        void log(log_level level, std::string_view message, std::string_view source, bool flush_)
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter == 0) return;

            const std::string_view resolved_source = source.empty() ? std::string_view{g_default_source} : source;

            std::string formatted_message;
            if (resolved_source.empty())
                formatted_message =
                    std::format("[{}] [{}]: {}", get_current_timestamp(), get_log_level_as_string(level), message);
            else
                formatted_message = std::format("[{}] [{}] [{}]: {}", get_current_timestamp(),
                                                get_log_level_as_string(level), resolved_source, message);

            write_to_outputs(formatted_message, level, flush_);
        }

        void initialize(log_level level,
                        const std::string &default_source,
                        bool enable_console,
                        bool enable_file,
                        const std::string &filename)
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter++ > 0)
                return write_to_outputs("[logcoe] Already initialized, ignoring new configurations");

            g_log_level = level;
            g_default_source = default_source;
            g_use_console = enable_console;
            if (g_use_console && !g_console_stream) g_console_stream = &std::cout;
            g_use_file = enable_file;

            if (filename != g_filename) g_filename = filename;

            if (g_filename == "logcoe.log") g_filename = get_default_filename();

            if (g_use_file && !filename.empty())
            {
                if (g_file_stream.is_open()) g_file_stream.close();

                std::filesystem::path filepath(g_filename);
                std::error_code ec;

                if (filepath.has_parent_path() && !std::filesystem::exists(filepath.parent_path(), ec))
                    std::filesystem::create_directories(filepath.parent_path(), ec);

                if (std::filesystem::exists(filepath, ec) && std::filesystem::is_regular_file(filepath, ec))
                    std::filesystem::remove(filepath, ec);

                g_file_stream.open(g_filename);
                if (!g_file_stream.is_open())
                {
                    write_to_outputs(std::format("[logcoe] ERROR: Failed to open log file: {}", g_filename));
                    g_use_file = false;
                }
            }

            write_to_outputs(std::format("[logcoe] Initialized, log level: {}", get_log_level_as_string(g_log_level)));
        }

        void flush()
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter == 0) return;
            flush_streams();
        }

        void shutdown()
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter == 0) return;
            if (--g_init_counter > 0) return;

            std::string shutdown_message = "[logcoe] shutting down";
            write_to_outputs(shutdown_message);

            flush_streams();
            if (g_file_stream.is_open()) g_file_stream.close();

            g_console_stream = nullptr;
            g_use_console = false;
            g_use_file = false;
            g_log_level = log_level::none;
            g_filename = "logcoe.log";
            g_init_counter = 0;
        }

        void set_log_level(log_level level)
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter == 0) return;

            g_log_level = level;
        }

        void set_console_output(std::ostream &stream)
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter == 0) return;

            if (g_use_console && g_console_stream) g_console_stream->flush();
            g_console_stream = &stream;
            g_use_console = true;
        }

        [[nodiscard]] std::expected<void, logcoe::error_reason> set_file_output(const std::string &filename)
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter == 0) return std::unexpected(logcoe::error_reason::not_initialized);

            if (g_file_stream.is_open())
            {
                if (g_use_file) g_file_stream.flush();
                g_file_stream.close();
            }

            g_filename = filename.empty() ? get_default_filename() : filename;
            g_use_file = true;

            g_file_stream.open(g_filename);
            if (!g_file_stream.is_open())
            {
                write_to_outputs(std::format("[logcoe] ERROR: Failed to open log file: {}", g_filename));
                g_use_file = false;
                return std::unexpected(logcoe::error_reason::file_open_failure);
            }

            return {};
        }

        void disable_console_output()
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter == 0) return;

            if (g_use_console && g_console_stream) g_console_stream->flush();
            g_console_stream = nullptr;
            g_use_console = false;
        }

        void disable_file_output()
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter == 0) return;

            if (g_file_stream.is_open())
            {
                if (g_use_file) g_file_stream.flush();
                g_file_stream.close();
            }

            g_filename = "logcoe.log";
            g_use_file = false;
        }

        [[nodiscard]] std::expected<void, logcoe::error_reason> set_time_format(const std::string &format_)
        {
            std::scoped_lock lock(g_mutex);
            if (g_init_counter == 0) return std::unexpected(logcoe::error_reason::not_initialized);

            auto now = std::chrono::system_clock::now();
            std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);
            std::tm tm_now;
    #ifdef _WIN32
            localtime_s(&tm_now, &time_t_now);
    #else
            localtime_r(&time_t_now, &tm_now);
    #endif

            char buffer[256];
            std::size_t result = std::strftime(buffer, sizeof(buffer), format_.c_str(), &tm_now);
            if (result == 0)
            {
                write_to_outputs(std::format(
                    "[logcoe] ERROR: Invalid time format provided: \"{}\". Keeping the current format", format_));
                return std::unexpected(logcoe::error_reason::invalid_time_format);
            }

            g_time_format = format_;

            return {};
        }

        bool is_initialized()
        {
            std::scoped_lock lock(g_mutex);

            return g_init_counter > 0;
        }

        log_level get_log_level()
        {
            std::scoped_lock lock(g_mutex);

            return g_log_level;
        }

        void debug(std::string_view message, std::string_view source, bool flush_)
        {
            log(log_level::debug, message, source, flush_);
        }

        void info(std::string_view message, std::string_view source, bool flush_)
        {
            log(log_level::info, message, source, flush_);
        }

        void warning(std::string_view message, std::string_view source, bool flush_)
        {
            log(log_level::warning, message, source, flush_);
        }

        void error(std::string_view message, std::string_view source, bool flush_)
        {
            log(log_level::error, message, source, flush_);
        }

    } // namespace internal
} // namespace
#endif

namespace logcoe
{
#ifdef NDEBUG
    void initialize(log_level, const std::string &, bool, bool, const std::string &) { }

    void shutdown() { }

    void set_log_level(log_level) { }
    void set_console_output(std::ostream &) { }
    std::expected<void, error_reason> set_file_output(const std::string &) { return {}; }
    void disable_console_output() { }
    void disable_file_output() { }
    std::expected<void, error_reason> set_time_format(const std::string &) { return {}; }

    bool is_initialized() { return false; }
    log_level get_log_level() { return log_level::none; }

    void debug(std::string_view, std::string_view, bool) { }
    void info(std::string_view, std::string_view, bool) { }
    void warning(std::string_view, std::string_view, bool) { }
    void error(std::string_view, std::string_view, bool) { }
    void flush() { }
#else
    void initialize(log_level level,
                    const std::string &default_source,
                    bool enable_console,
                    bool enable_file,
                    const std::string &filename)
    {
        internal::initialize(level, default_source, enable_console, enable_file, filename);
    }

    void shutdown() { internal::shutdown(); }

    void set_log_level(log_level level) { internal::set_log_level(level); }
    void set_console_output(std::ostream &stream) { internal::set_console_output(stream); }
    std::expected<void, error_reason> set_file_output(const std::string &filename)
    {
        return internal::set_file_output(filename);
    }
    void disable_console_output() { internal::disable_console_output(); }
    void disable_file_output() { internal::disable_file_output(); }
    std::expected<void, error_reason> set_time_format(const std::string &format)
    {
        return internal::set_time_format(format);
    }

    bool is_initialized() { return internal::is_initialized(); }
    log_level get_log_level() { return internal::get_log_level(); }

    void debug(std::string_view message, std::string_view source, bool flush_)
    {
        internal::debug(message, source, flush_);
    }
    void info(std::string_view message, std::string_view source, bool flush_)
    {
        internal::info(message, source, flush_);
    }
    void warning(std::string_view message, std::string_view source, bool flush_)
    {
        internal::warning(message, source, flush_);
    }
    void error(std::string_view message, std::string_view source, bool flush_)
    {
        internal::error(message, source, flush_);
    }
    void flush() { internal::flush(); }
#endif

} // namespace logcoe
