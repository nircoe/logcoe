#include <gtest/gtest.h>
#include <logcoe.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>
#include <regex>

class LogcoeTest : public ::testing::Test
{
protected:
    std::stringstream testStream;
    std::string testFilename;

    void SetUp() override
    {
        testFilename = "test_logfile_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".log";

        while(logcoe::is_initialized()) { logcoe::shutdown(); }
    }

    void TearDown() override
    {
        while(logcoe::is_initialized()) { logcoe::shutdown(); }

        if (std::filesystem::exists(testFilename))
            std::filesystem::remove(testFilename);
    }

    std::string readLogFile(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            return "";

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool matchesLogPattern(const std::string &text, logcoe::log_level level,
                           const std::string &message, const std::string &source = "")
    {
        std::string levelStr;
        switch (level)
        {
            case logcoe::log_level::debug:
                levelStr = "DEBUG";
                break;
            case logcoe::log_level::info:
                levelStr = "INFO";
                break;
            case logcoe::log_level::warning:
                levelStr = "WARNING";
                break;
            case logcoe::log_level::error:
                levelStr = "ERROR";
                break;
            default:
                levelStr = "NONE";
                break;
        }

        std::string pattern = "\\[.*?\\] \\[" + levelStr + "\\]";

        if (!source.empty())
            pattern += " \\[" + source + "\\]";

        pattern += ": " + message;

        std::regex re(pattern);
        return std::regex_search(text, re);
    }
};

TEST_F(LogcoeTest, DefaultInitialization)
{
    logcoe::initialize();

    EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::debug);

    logcoe::info("Test info message");
}

TEST_F(LogcoeTest, CustomInitialization)
{
    logcoe::initialize(logcoe::log_level::info, "", true, true, testFilename);

    EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::info);

    logcoe::info("Test debug message");

    EXPECT_TRUE(std::filesystem::exists(testFilename));

    std::string fileContent = readLogFile(testFilename);
    EXPECT_TRUE(matchesLogPattern(fileContent, logcoe::log_level::info, "Test debug message"));
}

TEST_F(LogcoeTest, LogLevelFiltering)
{
    logcoe::initialize(logcoe::log_level::warning);
    logcoe::set_console_output(testStream);

    logcoe::debug("Debug message");
    logcoe::info("Info message");

    logcoe::warning("Warning message");
    logcoe::error("Error message");

    std::string output = testStream.str();

    EXPECT_FALSE(matchesLogPattern(output, logcoe::log_level::debug, "Debug message"));
    EXPECT_FALSE(matchesLogPattern(output, logcoe::log_level::info, "Info message"));

    EXPECT_TRUE(matchesLogPattern(output, logcoe::log_level::warning, "Warning message"));
    EXPECT_TRUE(matchesLogPattern(output, logcoe::log_level::error, "Error message"));
}

TEST_F(LogcoeTest, ChangeLogLevel)
{
    logcoe::initialize(logcoe::log_level::error);
    EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::error);

    logcoe::set_log_level(logcoe::log_level::debug);
    EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::debug);
}

TEST_F(LogcoeTest, ConsoleRedirection)
{
    logcoe::initialize();
    logcoe::set_console_output(testStream);

    logcoe::info("Test message");

    std::string output = testStream.str();
    EXPECT_TRUE(matchesLogPattern(output, logcoe::log_level::info, "Test message"));
}

TEST_F(LogcoeTest, FileOutput)
{
    logcoe::initialize();

    EXPECT_TRUE(logcoe::set_file_output(testFilename));

    logcoe::info("File test message");

    std::string fileContent = readLogFile(testFilename);
    EXPECT_TRUE(matchesLogPattern(fileContent, logcoe::log_level::info, "File test message"));

    logcoe::disable_file_output();
    logcoe::info("This shouldn't be in the file");

    fileContent = readLogFile(testFilename);
    EXPECT_FALSE(matchesLogPattern(fileContent, logcoe::log_level::info, "This shouldn't be in the file"));
}

TEST_F(LogcoeTest, DisableConsole)
{
    logcoe::initialize();
    logcoe::set_console_output(testStream);

    logcoe::info("Before disable");

    logcoe::disable_console_output();
    logcoe::info("After disable");

    std::string output = testStream.str();
    EXPECT_TRUE(matchesLogPattern(output, logcoe::log_level::info, "Before disable"));
    EXPECT_FALSE(matchesLogPattern(output, logcoe::log_level::info, "After disable"));
}

TEST_F(LogcoeTest, SourceField)
{
    logcoe::initialize();
    logcoe::set_console_output(testStream);

    logcoe::info("Message with source", "TestSource");

    std::string output = testStream.str();
    EXPECT_TRUE(matchesLogPattern(output, logcoe::log_level::info, "Message with source", "TestSource"));
}

TEST_F(LogcoeTest, TimeFormat)
{
    logcoe::initialize();
    logcoe::set_console_output(testStream);

    logcoe::set_time_format("%H:%M:%S");

    logcoe::info("Custom time format");

    std::string output = testStream.str();
    std::regex timePattern("\\[\\d{2}:\\d{2}:\\d{2}\\]");
    EXPECT_TRUE(std::regex_search(output, timePattern));
}