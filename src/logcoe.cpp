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
    class LoggerImpl
    {
        static unsigned int s_initCounter;
        static log_level s_logLevel;
        static std::string s_defaultSource;
        static std::mutex s_mutex;
        static std::string s_filename;
        static std::ofstream s_fileStream;
        static std::ostream *s_consoleStream;
        static bool s_useFile;
        static bool s_useConsole;
        static std::string s_timeFormat;

        static std::string getCurrentTimestamp();
        static std::string getLogLevelAsString(log_level level);
        static void writeToOutputs(const std::string &formattedMessage,
                                   log_level level = log_level::info,
                                   bool flush_ = true);
        static void log(log_level level, const std::string &message, const std::string &source, bool flush_);

    public:
        static void initialize(log_level level = log_level::info,
                               const std::string &default_source = "",
                               bool enable_console = true,
                               bool enable_file = false,
                               const std::string &filename = "logcoe.log");
        static void shutdown();

        static void setLogLevel(log_level level);
        static void setConsoleOutput(std::ostream &stream);
        static bool setFileOutput(const std::string &filename);
        static void disableConsoleOutput();
        static void disableFileOutput();
        static void setTimeFormat(const std::string &format);

        static bool isInitialized();
        static log_level getLogLevel();

        static void debug(const std::string &message, const std::string &source = "", bool flush_ = true);
        static void info(const std::string &message, const std::string &source = "", bool flush_ = true);
        static void warning(const std::string &message, const std::string &source = "", bool flush_ = true);
        static void error(const std::string &message, const std::string &source = "", bool flush_ = true);
        static void flush();
    };

    unsigned int LoggerImpl::s_initCounter = 0;
    log_level LoggerImpl::s_logLevel = log_level::info;
    std::string LoggerImpl::s_defaultSource = "";
    std::mutex LoggerImpl::s_mutex;
    std::string LoggerImpl::s_filename = "logcoe.log";
    std::ofstream LoggerImpl::s_fileStream;
    std::ostream *LoggerImpl::s_consoleStream = &std::cout;
    bool LoggerImpl::s_useFile = false;
    bool LoggerImpl::s_useConsole = true;
    std::string LoggerImpl::s_timeFormat = "%d/%m/%Y__%H:%M:%S";

    std::string LoggerImpl::getCurrentTimestamp()
    {
        if(s_initCounter == 0) return "";

        auto now = std::chrono::system_clock::now();
        std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);

        std::tm tm_now;
#ifdef _WIN32
        localtime_s(&tm_now, &time_t_now);
#else
        localtime_r(&time_t_now, &tm_now);
#endif

        char buffer[256];
        std::strftime(buffer, sizeof(buffer), s_timeFormat.c_str(), &tm_now);

        return std::string(buffer);
    }

    std::string LoggerImpl::getLogLevelAsString(log_level level)
    {
        if(s_initCounter == 0) return "";

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

    void LoggerImpl::writeToOutputs(const std::string &formattedMessage, log_level level, bool flush_)
    {
        if (s_initCounter == 0 || static_cast<int>(level) < static_cast<int>(s_logLevel))
            return;

        if (s_useConsole && s_consoleStream)
        {
            *s_consoleStream << formattedMessage << std::endl;
            if (flush_)
                s_consoleStream->flush();
        }

        if (s_useFile)
        {
            s_fileStream << formattedMessage << std::endl;
            if (flush_)
                s_fileStream.flush();
        }
    }

    void LoggerImpl::log(log_level level, const std::string &message, const std::string &source, bool flush_)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if(s_initCounter == 0) return;

        std::stringstream formattedMessage;
        formattedMessage << "[" << getCurrentTimestamp() << "] ";
        formattedMessage << "[" << getLogLevelAsString(level) << "]";
        if (!source.empty())
            formattedMessage << " [" << source << "]";
        formattedMessage << ": " << message;

        writeToOutputs(formattedMessage.str(), level, flush_);
    }

    void LoggerImpl::initialize(log_level level, const std::string &default_source, bool enable_console, bool enable_file, const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if(s_initCounter++ > 0)
            return writeToOutputs("[logcoe] Already initialized, ignoring new configurations");

        s_logLevel = level;
        s_defaultSource = default_source;
        s_useConsole = enable_console;
        if (s_useConsole && !s_consoleStream)
            s_consoleStream = &std::cout;
        s_useFile = enable_file;

        if (filename != s_filename)
            s_filename = filename;

        if (s_filename == "logcoe.log")
            s_filename = "logcoe_" + getCurrentTimestamp() + ".log";

        if (s_useFile && !filename.empty())
        {
            if (s_fileStream.is_open())
                s_fileStream.close();

            std::filesystem::path filepath(s_filename);

            if (filepath.has_parent_path() && !std::filesystem::exists(filepath.parent_path()))
                std::filesystem::create_directories(filepath.parent_path());
            
            if (std::filesystem::exists(filepath) && std::filesystem::is_regular_file(filepath))
                std::filesystem::remove(filepath);

            s_fileStream.open(s_filename);
            if (!s_fileStream.is_open())
            {
                writeToOutputs("[logcoe] ERROR: Failed to open log file: " + s_filename);
                s_useFile = false;
            }
        }

        writeToOutputs("[logcoe] Initialized, log level: " + getLogLevelAsString(s_logLevel));
    }

    void LoggerImpl::shutdown()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if (--s_initCounter > 0) return;

        std::string shutdownMessage = "[logcoe] shutting down";
        writeToOutputs(shutdownMessage);

        flush();
        if (s_fileStream.is_open())
            s_fileStream.close();

        s_consoleStream = nullptr;
        s_useConsole = false;
        s_useFile = false;
        s_logLevel = log_level::none;
        s_filename = "logcoe.log";
        s_initCounter = 0;
    }

    void LoggerImpl::setLogLevel(log_level level)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if(s_initCounter == 0) return;

        s_logLevel = level;
    }

    void LoggerImpl::setConsoleOutput(std::ostream &stream)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if(s_initCounter == 0) return;

        if (s_useConsole && s_consoleStream)
            s_consoleStream->flush();
        s_consoleStream = &stream;
        s_useConsole = true;
    }

    bool LoggerImpl::setFileOutput(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if(s_initCounter == 0) return false;

        if (s_fileStream.is_open())
        {
            if (s_useFile)
                s_fileStream.flush();
            s_fileStream.close();
        }

        s_filename = filename.empty() ? "logcoe_" + getCurrentTimestamp() + ".log" : filename;
        s_useFile = true;

        s_fileStream.open(s_filename);
        if (!s_fileStream.is_open())
        {
            writeToOutputs("[logcoe] ERROR: Failed to open log file: " + s_filename);
            s_useFile = false;
        }

        return s_useFile;
    }

    void LoggerImpl::disableConsoleOutput()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if(s_initCounter == 0) return;

        if (s_useConsole && s_consoleStream)
            s_consoleStream->flush();
        s_consoleStream = nullptr;
        s_useConsole = false;
    }

    void LoggerImpl::disableFileOutput()
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if(s_initCounter == 0) return;

        if (s_fileStream.is_open())
        {
            if (s_useFile)
                s_fileStream.flush();
            s_fileStream.close();
        }

        s_filename = "logcoe.log";
        s_useFile = false;
    }

    void LoggerImpl::setTimeFormat(const std::string &format)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        if(s_initCounter == 0) return;

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
                writeToOutputs("[logcoe] ERROR: Invalid time format provided: \"" + format + "\". Keeping the current format");
                return;
            }

            s_timeFormat = format;
        }
        catch (const std::exception &e)
        {
            std::stringstream message;
            message << "[logcoe] ERROR: Exception while validating time format: " << e.what();
            writeToOutputs(message.str());
        }
    }

    bool LoggerImpl::isInitialized()
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        return s_initCounter > 0;
    }

    log_level LoggerImpl::getLogLevel()
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        return s_logLevel;
    }

    void LoggerImpl::debug(const std::string &message, const std::string &source, bool flush_)
    {
        log(log_level::debug, message, source.empty() ? s_defaultSource : source, flush_);
    }

    void LoggerImpl::info(const std::string &message, const std::string &source, bool flush_)
    {
        log(log_level::info, message, source.empty() ? s_defaultSource : source, flush_);
    }

    void LoggerImpl::warning(const std::string &message, const std::string &source, bool flush_)
    {
        log(log_level::warning, message, source.empty() ? s_defaultSource : source, flush_);
    }

    void LoggerImpl::error(const std::string &message, const std::string &source, bool flush_)
    {
        log(log_level::error, message, source.empty() ? s_defaultSource : source, flush_);
    }

    void LoggerImpl::flush()
    {
        if(s_initCounter == 0) return;
        if (s_useConsole && s_consoleStream)
            s_consoleStream->flush();
        if (s_useFile && s_fileStream.is_open())
            s_fileStream.flush();
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
                    bool enable_file, const std::string &filename) { LoggerImpl::initialize(level, default_source, enable_console, enable_file, filename); }

    void shutdown() { LoggerImpl::shutdown(); }

    void set_log_level(log_level level) { LoggerImpl::setLogLevel(level); }
    void set_console_output(std::ostream &stream) { LoggerImpl::setConsoleOutput(stream); }
    bool set_file_output(const std::string &filename) { return LoggerImpl::setFileOutput(filename); }
    void disable_console_output() { LoggerImpl::disableConsoleOutput(); }
    void disable_file_output() { LoggerImpl::disableFileOutput(); }
    void set_time_format(const std::string &format) { LoggerImpl::setTimeFormat(format); }

    bool is_initialized() { return LoggerImpl::isInitialized(); }
    log_level get_log_level() { return LoggerImpl::getLogLevel(); }

    void debug(const std::string &message, const std::string &source, bool flush_) { LoggerImpl::debug(message, source, flush_); }
    void info(const std::string &message, const std::string &source, bool flush_) { LoggerImpl::info(message, source, flush_); }
    void warning(const std::string &message, const std::string &source, bool flush_) { LoggerImpl::warning(message, source, flush_); }
    void error(const std::string &message, const std::string &source, bool flush_) { LoggerImpl::error(message, source, flush_); }
    void flush() { LoggerImpl::flush(); }
#endif

} // namespace logcoe