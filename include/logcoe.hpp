#pragma once

#include <string>

namespace logcoe
{
    enum class LogLevel
    {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        NONE
    };

    void initialize(LogLevel level = LogLevel::INFO,
                    const std::string &defaultSource = "",
                    bool enableConsole = true,
                    bool enableFile = false,
                    const std::string &filename = "logcoe.log");
    void shutdown();

    void setLogLevel(LogLevel level);
    void setConsoleOutput(std::ostream &stream);
    bool setFileOutput(const std::string &filename);
    void disableConsoleOutput();
    void disableFileOutput();
    void setTimeFormat(const std::string &format);

    bool isInitialized();
    LogLevel getLogLevel();

    void debug(const std::string &message, const std::string &source = "", bool flush = true);
    void info(const std::string &message, const std::string &source = "", bool flush = true);
    void warning(const std::string &message, const std::string &source = "", bool flush = true);
    void error(const std::string &message, const std::string &source = "", bool flush = true);
    void flush();

} // namespace logcoe