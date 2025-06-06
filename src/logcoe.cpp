#include <logcoe.hpp>
#include <chrono>
#include <exception>
#include <sstream>
#include <mutex>
#include <iostream>
#include <fstream>

using logcoe::LogLevel;

namespace
{
    class LoggerImpl
    {
        static LogLevel s_logLevel;
        static std::mutex s_mutex;
        static std::string s_filename;
        static std::ofstream s_fileStream;
        static std::ostream *s_consoleStream;
        static bool s_useFile;
        static bool s_useConsole;
        static std::string s_timeFormat;

        static std::string getCurrentTimestamp();
        static std::string getLogLevelAsString(LogLevel level);
        static void writeToOutputs(const std::string &formattedMessage,
                                   LogLevel level = LogLevel::INFO,
                                   bool flush = true);
        static void log(LogLevel level, const std::string &message, const std::string &source, bool flush);

    public:
        static void initialize(LogLevel level = LogLevel::INFO,
                               bool enableConsole = true,
                               bool enableFile = false,
                               const std::string &filename = "logcoe.log");
        static void shutdown();

        static void setLogLevel(LogLevel level);
        static void setConsoleOutput(std::ostream &stream);
        static bool setFileOutput(const std::string &filename);
        static void disableConsoleOutput();
        static void disableFileOutput();
        static void setTimeFormat(const std::string &format);

        static LogLevel getLogLevel();

        static void debug(const std::string &message, const std::string &source = "", bool flush = true);
        static void info(const std::string &message, const std::string &source = "", bool flush = true);
        static void warning(const std::string &message, const std::string &source = "", bool flush = true);
        static void error(const std::string &message, const std::string &source = "", bool flush = true);
        static void flush();
    };

    LogLevel LoggerImpl::s_logLevel = LogLevel::INFO;
    std::mutex LoggerImpl::s_mutex;
    std::string LoggerImpl::s_filename = "logcoe.log";
    std::ofstream LoggerImpl::s_fileStream;
    std::ostream *LoggerImpl::s_consoleStream = &std::cout;
    bool LoggerImpl::s_useFile = false;
    bool LoggerImpl::s_useConsole = true;
    std::string LoggerImpl::s_timeFormat = "%Y-%m-%d_%H-%M-%S";

    std::string LoggerImpl::getCurrentTimestamp()
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
        std::strftime(buffer, sizeof(buffer), s_timeFormat.c_str(), &tm_now);

        return std::string(buffer);
    }

    std::string LoggerImpl::getLogLevelAsString(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::DEBUG:
            return "DEBUG";
        case LogLevel::INFO:
            return "INFO";
        case LogLevel::WARNING:
            return "WARNING";
        case LogLevel::ERROR:
            return "ERROR";
        default:
            return "NONE";
        }
    }

    void LoggerImpl::writeToOutputs(const std::string &formattedMessage, LogLevel level, bool flush)
    {
        if (static_cast<int>(level) < static_cast<int>(s_logLevel))
            return;

        if (s_useConsole && s_consoleStream)
        {
            *s_consoleStream << formattedMessage << std::endl;
            if (flush)
                s_consoleStream->flush();
        }

        if (s_useFile)
        {
            s_fileStream << formattedMessage << std::endl;
            if (flush)
                s_fileStream.flush();
        }
    }

    void LoggerImpl::log(LogLevel level, const std::string &message, const std::string &source, bool flush)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        std::stringstream formattedMessage;
        formattedMessage << "[" << getCurrentTimestamp() << "] ";
        formattedMessage << "[" << getLogLevelAsString(level) << "]";
        if (!source.empty())
            formattedMessage << " [" << source << "]";
        formattedMessage << ": " << message;

        writeToOutputs(formattedMessage.str(), level, flush);
    }

    void LoggerImpl::initialize(LogLevel level, bool enableConsole, bool enableFile, const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        s_logLevel = level;
        s_useConsole = enableConsole;
        if (s_useConsole && !s_consoleStream)
            s_consoleStream = &std::cout;
        s_useFile = enableFile;

        if (filename != s_filename)
            s_filename = filename;

        if (s_filename == "logcoe.log")
            s_filename = "logcoe_" + getCurrentTimestamp() + ".log";

        if (s_useFile && !filename.empty())
        {
            if (s_fileStream.is_open())
                s_fileStream.close();

            s_fileStream.open(s_filename);
            if (!s_fileStream.is_open())
            {
                writeToOutputs("ERROR: Failed to open log file: " + s_filename);
                s_useFile = false;
            }
        }

        writeToOutputs("Logger initialized with level: " + getLogLevelAsString(s_logLevel));
    }

    void LoggerImpl::shutdown()
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        std::string shutdownMessage = "Logger shutting down";
        writeToOutputs(shutdownMessage);

        flush();
        if (s_fileStream.is_open())
            s_fileStream.close();

        s_consoleStream = nullptr;
        s_useConsole = false;
        s_useFile = false;
        s_logLevel = LogLevel::NONE;
        s_filename = "logcoe.log";
    }

    void LoggerImpl::setLogLevel(LogLevel level)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        s_logLevel = level;
    }

    void LoggerImpl::setConsoleOutput(std::ostream &stream)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        if (s_useConsole && s_consoleStream)
            s_consoleStream->flush();
        s_consoleStream = &stream;
        s_useConsole = true;
    }

    bool LoggerImpl::setFileOutput(const std::string &filename)
    {
        std::lock_guard<std::mutex> lock(s_mutex);

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
            writeToOutputs("ERROR: Failed to open log file: " + s_filename);
            s_useFile = false;
        }

        return s_useFile;
    }

    void LoggerImpl::disableConsoleOutput()
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        if (s_useConsole && s_consoleStream)
            s_consoleStream->flush();
        s_consoleStream = nullptr;
        s_useConsole = false;
    }

    void LoggerImpl::disableFileOutput()
    {
        std::lock_guard<std::mutex> lock(s_mutex);

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
                writeToOutputs("ERROR: Invalid time format provided: \"" + format + "\". Keeping the current format");
                return;
            }

            s_timeFormat = format;
        }
        catch (const std::exception &e)
        {
            std::stringstream message;
            message << "ERROR: Exception while validating time format: " << e.what();
            writeToOutputs(message.str());
        }
    }

    LogLevel LoggerImpl::getLogLevel()
    {
        std::lock_guard<std::mutex> lock(s_mutex);

        return s_logLevel;
    }

    void LoggerImpl::debug(const std::string &message, const std::string &source, bool flush)
    {
        log(LogLevel::DEBUG, message, source, flush);
    }

    void LoggerImpl::info(const std::string &message, const std::string &source, bool flush)
    {
        log(LogLevel::INFO, message, source, flush);
    }

    void LoggerImpl::warning(const std::string &message, const std::string &source, bool flush)
    {
        log(LogLevel::WARNING, message, source, flush);
    }

    void LoggerImpl::error(const std::string &message, const std::string &source, bool flush)
    {
        log(LogLevel::ERROR, message, source, flush);
    }

    void LoggerImpl::flush()
    {
        if (s_useConsole && s_consoleStream)
            s_consoleStream->flush();
        if (s_useFile && s_fileStream.is_open())
            s_fileStream.flush();
    }
}

namespace logcoe
{
    void initialize(LogLevel level, bool enableConsole,
                    bool enableFile, const std::string &filename) { LoggerImpl::initialize(level, enableConsole, enableFile, filename); }

    void shutdown() { LoggerImpl::shutdown(); }

    void setLogLevel(LogLevel level) { LoggerImpl::setLogLevel(level); }
    void setConsoleOutput(std::ostream &stream) { LoggerImpl::setConsoleOutput(stream); }
    bool setFileOutput(const std::string &filename) { return LoggerImpl::setFileOutput(filename); }
    void disableConsoleOutput() { LoggerImpl::disableConsoleOutput(); }
    void disableFileOutput() { LoggerImpl::disableFileOutput(); }
    void setTimeFormat(const std::string &format) { LoggerImpl::setTimeFormat(format); }

    LogLevel getLogLevel() { return LoggerImpl::getLogLevel(); }

    void debug(const std::string &message, const std::string &source, bool flush) { LoggerImpl::debug(message, source, flush); }
    void info(const std::string &message, const std::string &source, bool flush) { LoggerImpl::info(message, source, flush); }
    void warning(const std::string &message, const std::string &source, bool flush) { LoggerImpl::warning(message, source, flush); }
    void error(const std::string &message, const std::string &source, bool flush) { LoggerImpl::error(message, source, flush); }
    void flush() { LoggerImpl::flush(); }

} // namespace logcoe