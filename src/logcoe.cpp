#include <logcoe.hpp>
#include <chrono>
#include <exception>
#include <sstream>
#include <mutex>
#include <iostream>
#include <fstream>
#include <filesystem>

using logcoe::log_level;

#ifndef NDEBUG
namespace
{
    unsigned int  g_init_counter = 0;
    log_level     g_log_level = log_level::info;
    std::string   g_default_source = "";
    std::mutex    g_mutex;
    std::string   g_filename = "logcoe.log";
    std::ofstream g_file_stream;
    std::ostream *g_console_stream = &std::cout;
    bool          g_use_file = false;
    bool          g_use_console = true;
    std::string   g_time_format = "%d/%m/%Y__%H:%M:%S";

    std::string get_current_timestamp()
    {
        if(g_init_counter == 0) return "";

        auto now = std::chrono::system_clock::now();
        std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);

        std::tm tm_now;
#ifdef _WIN32
        localtime_s(&tm_now, &time_t_now);
#else
        localtime_r(&time_t_now, &tm_now);
#endif

        char buffer[256];
        std::strftime(buffer, sizeof(buffer), g_time_format.c_str(), &tm_now);

        return std::string(buffer);
    }

    std::string get_log_level_as_string(log_level level)
    {
        if(g_init_counter == 0) return "";

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
        default:
            return "NONE";
        }
    }

    void write_to_outputs(const std::string &formatted_message, log_level level = log_level::info, bool flush_ = true)
    {
        if (g_init_counter == 0 || static_cast<int>(level) < static_cast<int>(g_log_level))
            return;

        if (g_use_console && g_console_stream)
        {
            *g_console_stream << formatted_message << std::endl;
            if (flush_)
                g_console_stream->flush();
        }

        if (g_use_file)
        {
            g_file_stream << formatted_message << std::endl;
            if (flush_)
                g_file_stream.flush();
        }
    }

    void log(log_level level, const std::string &message, const std::string &source, bool flush_)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if(g_init_counter == 0) return;

        std::stringstream formatted_message;
        formatted_message << "[" << get_current_timestamp() << "] ";
        formatted_message << "[" << get_log_level_as_string(level) << "]";
        if (!source.empty())
            formatted_message << " [" << source << "]";
        formatted_message << ": " << message;

        write_to_outputs(formatted_message.str(), level, flush_);
    }

    void initialize(log_level level,
                    const std::string &default_source,
                    bool enable_console,
                    bool enable_file,
                    const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if(g_init_counter++ > 0)
            return write_to_outputs("[logcoe] Already initialized, ignoring new configurations");

        g_log_level = level;
        g_default_source = default_source;
        g_use_console = enable_console;
        if (g_use_console && !g_console_stream)
            g_console_stream = &std::cout;
        g_use_file = enable_file;

        if (filename != g_filename)
            g_filename = filename;

        if (g_filename == "logcoe.log")
            g_filename = "logcoe_" + get_current_timestamp() + ".log";

        if (g_use_file && !filename.empty())
        {
            if (g_file_stream.is_open())
                g_file_stream.close();

            std::filesystem::path filepath(g_filename);

            if (filepath.has_parent_path() && !std::filesystem::exists(filepath.parent_path()))
                std::filesystem::create_directories(filepath.parent_path());

            if (std::filesystem::exists(filepath) && std::filesystem::is_regular_file(filepath))
                std::filesystem::remove(filepath);

            g_file_stream.open(g_filename);
            if (!g_file_stream.is_open())
            {
                write_to_outputs("[logcoe] ERROR: Failed to open log file: " + g_filename);
                g_use_file = false;
            }
        }

        write_to_outputs("[logcoe] Initialized, log level: " + get_log_level_as_string(g_log_level));
    }

    void flush()
    {
        if(g_init_counter == 0) return;
        if (g_use_console && g_console_stream)
            g_console_stream->flush();
        if (g_use_file && g_file_stream.is_open())
            g_file_stream.flush();
    }

    void shutdown()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if (--g_init_counter > 0) return;

        std::string shutdown_message = "[logcoe] shutting down";
        write_to_outputs(shutdown_message);

        flush();
        if (g_file_stream.is_open())
            g_file_stream.close();

        g_console_stream = nullptr;
        g_use_console = false;
        g_use_file = false;
        g_log_level = log_level::none;
        g_filename = "logcoe.log";
        g_init_counter = 0;
    }

    void set_log_level(log_level level)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if(g_init_counter == 0) return;

        g_log_level = level;
    }

    void set_console_output(std::ostream &stream)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if(g_init_counter == 0) return;

        if (g_use_console && g_console_stream)
            g_console_stream->flush();
        g_console_stream = &stream;
        g_use_console = true;
    }

    bool set_file_output(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if(g_init_counter == 0) return false;

        if (g_file_stream.is_open())
        {
            if (g_use_file)
                g_file_stream.flush();
            g_file_stream.close();
        }

        g_filename = filename.empty() ? "logcoe_" + get_current_timestamp() + ".log" : filename;
        g_use_file = true;

        g_file_stream.open(g_filename);
        if (!g_file_stream.is_open())
        {
            write_to_outputs("[logcoe] ERROR: Failed to open log file: " + g_filename);
            g_use_file = false;
        }

        return g_use_file;
    }

    void disable_console_output()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if(g_init_counter == 0) return;

        if (g_use_console && g_console_stream)
            g_console_stream->flush();
        g_console_stream = nullptr;
        g_use_console = false;
    }

    void disable_file_output()
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if(g_init_counter == 0) return;

        if (g_file_stream.is_open())
        {
            if (g_use_file)
                g_file_stream.flush();
            g_file_stream.close();
        }

        g_filename = "logcoe.log";
        g_use_file = false;
    }

    void set_time_format(const std::string &format)
    {
        std::lock_guard<std::mutex> lock(g_mutex);
        if(g_init_counter == 0) return;

        try
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
            std::size_t result = std::strftime(buffer, sizeof(buffer), format.c_str(), &tm_now);
            if (result == 0)
            {
                write_to_outputs("[logcoe] ERROR: Invalid time format provided: \"" + format + "\". Keeping the current format");
                return;
            }

            g_time_format = format;
        }
        catch (const std::exception &e)
        {
            std::stringstream message;
            message << "[logcoe] ERROR: Exception while validating time format: " << e.what();
            write_to_outputs(message.str());
        }
    }

    bool is_initialized()
    {
        std::lock_guard<std::mutex> lock(g_mutex);

        return g_init_counter > 0;
    }

    log_level get_log_level()
    {
        std::lock_guard<std::mutex> lock(g_mutex);

        return g_log_level;
    }

    void debug(const std::string &message, const std::string &source, bool flush_)
    {
        log(log_level::debug, message, source.empty() ? g_default_source : source, flush_);
    }

    void info(const std::string &message, const std::string &source, bool flush_)
    {
        log(log_level::info, message, source.empty() ? g_default_source : source, flush_);
    }

    void warning(const std::string &message, const std::string &source, bool flush_)
    {
        log(log_level::warning, message, source.empty() ? g_default_source : source, flush_);
    }

    void error(const std::string &message, const std::string &source, bool flush_)
    {
        log(log_level::error, message, source.empty() ? g_default_source : source, flush_);
    }
}
#endif

namespace logcoe
{
#ifdef NDEBUG
    void initialize(log_level, const std::string &, bool, bool, const std::string &) { }

    void shutdown() { }

    void set_log_level(log_level) { }
    void set_console_output(std::ostream &) { }
    bool set_file_output(const std::string &) { return false; }
    void disable_console_output() { }
    void disable_file_output() { }
    void set_time_format(const std::string &) { }

    bool is_initialized() { return false; }
    log_level get_log_level() { return log_level::none; }

    void debug(const std::string &, const std::string &, bool) { }
    void info(const std::string &, const std::string &, bool) { }
    void warning(const std::string &, const std::string &, bool) { }
    void error(const std::string &, const std::string &, bool) { }
    void flush() { }
#else
    void initialize(log_level level, const std::string &default_source, bool enable_console,
                    bool enable_file, const std::string &filename) { ::initialize(level, default_source, enable_console, enable_file, filename); }

    void shutdown() { ::shutdown(); }

    void set_log_level(log_level level) { ::set_log_level(level); }
    void set_console_output(std::ostream &stream) { ::set_console_output(stream); }
    bool set_file_output(const std::string &filename) { return ::set_file_output(filename); }
    void disable_console_output() { ::disable_console_output(); }
    void disable_file_output() { ::disable_file_output(); }
    void set_time_format(const std::string &format) { ::set_time_format(format); }

    bool is_initialized() { return ::is_initialized(); }
    log_level get_log_level() { return ::get_log_level(); }

    void debug(const std::string &message, const std::string &source, bool flush_) { ::debug(message, source, flush_); }
    void info(const std::string &message, const std::string &source, bool flush_) { ::info(message, source, flush_); }
    void warning(const std::string &message, const std::string &source, bool flush_) { ::warning(message, source, flush_); }
    void error(const std::string &message, const std::string &source, bool flush_) { ::error(message, source, flush_); }
    void flush() { ::flush(); }
#endif

} // namespace logcoe