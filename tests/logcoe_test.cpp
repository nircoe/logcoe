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
        logcoe::shutdown();
    }

    void TearDown() override
    {
        logcoe::shutdown();

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

    bool matchesLogPattern(const std::string &text, logcoe::LogLevel level,
                           const std::string &message, const std::string &source = "")
    {
        std::string levelStr;
        switch (level)
        {
            case logcoe::LogLevel::DEBUG:
                levelStr = "DEBUG";
                break;
            case logcoe::LogLevel::INFO:
                levelStr = "INFO";
                break;
            case logcoe::LogLevel::WARNING:
                levelStr = "WARNING";
                break;
            case logcoe::LogLevel::ERROR:
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

    EXPECT_EQ(logcoe::getLogLevel(), logcoe::LogLevel::INFO);

    logcoe::info("Test info message");
}

TEST_F(LogcoeTest, CustomInitialization)
{
    logcoe::initialize(logcoe::LogLevel::DEBUG, true, true, testFilename);

    EXPECT_EQ(logcoe::getLogLevel(), logcoe::LogLevel::DEBUG);

    logcoe::debug("Test debug message");

    EXPECT_TRUE(std::filesystem::exists(testFilename));

    std::string fileContent = readLogFile(testFilename);
    EXPECT_TRUE(matchesLogPattern(fileContent, logcoe::LogLevel::DEBUG, "Test debug message"));
}

TEST_F(LogcoeTest, LogLevelFiltering)
{
    logcoe::initialize(logcoe::LogLevel::WARNING);
    logcoe::setConsoleOutput(testStream);

    logcoe::debug("Debug message");
    logcoe::info("Info message");

    logcoe::warning("Warning message");
    logcoe::error("Error message");

    std::string output = testStream.str();

    EXPECT_FALSE(matchesLogPattern(output, logcoe::LogLevel::DEBUG, "Debug message"));
    EXPECT_FALSE(matchesLogPattern(output, logcoe::LogLevel::INFO, "Info message"));

    EXPECT_TRUE(matchesLogPattern(output, logcoe::LogLevel::WARNING, "Warning message"));
    EXPECT_TRUE(matchesLogPattern(output, logcoe::LogLevel::ERROR, "Error message"));
}

TEST_F(LogcoeTest, ChangeLogLevel)
{
    logcoe::initialize(logcoe::LogLevel::ERROR);
    EXPECT_EQ(logcoe::getLogLevel(), logcoe::LogLevel::ERROR);

    logcoe::setLogLevel(logcoe::LogLevel::DEBUG);
    EXPECT_EQ(logcoe::getLogLevel(), logcoe::LogLevel::DEBUG);
}

TEST_F(LogcoeTest, ConsoleRedirection)
{
    logcoe::initialize();
    logcoe::setConsoleOutput(testStream);

    logcoe::info("Test message");

    std::string output = testStream.str();
    EXPECT_TRUE(matchesLogPattern(output, logcoe::LogLevel::INFO, "Test message"));
}

TEST_F(LogcoeTest, FileOutput)
{
    logcoe::initialize();

    EXPECT_TRUE(logcoe::setFileOutput(testFilename));

    logcoe::info("File test message");

    std::string fileContent = readLogFile(testFilename);
    EXPECT_TRUE(matchesLogPattern(fileContent, logcoe::LogLevel::INFO, "File test message"));

    logcoe::disableFileOutput();
    logcoe::info("This shouldn't be in the file");

    fileContent = readLogFile(testFilename);
    EXPECT_FALSE(matchesLogPattern(fileContent, logcoe::LogLevel::INFO, "This shouldn't be in the file"));
}

TEST_F(LogcoeTest, DisableConsole)
{
    logcoe::initialize();
    logcoe::setConsoleOutput(testStream);

    logcoe::info("Before disable");

    logcoe::disableConsoleOutput();
    logcoe::info("After disable");

    std::string output = testStream.str();
    EXPECT_TRUE(matchesLogPattern(output, logcoe::LogLevel::INFO, "Before disable"));
    EXPECT_FALSE(matchesLogPattern(output, logcoe::LogLevel::INFO, "After disable"));
}

TEST_F(LogcoeTest, SourceField)
{
    logcoe::initialize();
    logcoe::setConsoleOutput(testStream);

    logcoe::info("Message with source", "TestSource");

    std::string output = testStream.str();
    EXPECT_TRUE(matchesLogPattern(output, logcoe::LogLevel::INFO, "Message with source", "TestSource"));
}

TEST_F(LogcoeTest, TimeFormat)
{
    logcoe::initialize();
    logcoe::setConsoleOutput(testStream);

    logcoe::setTimeFormat("%H:%M:%S");

    logcoe::info("Custom time format");

    std::string output = testStream.str();
    std::regex timePattern("\\[\\d{2}:\\d{2}:\\d{2}\\]");
    EXPECT_TRUE(std::regex_search(output, timePattern));
}