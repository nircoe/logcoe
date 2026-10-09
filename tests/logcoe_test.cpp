#include <logcoe.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>

class LogcoeTest : public ::testing::Test
{
protected:
    std::stringstream m_test_stream;
    std::string m_test_filename;

    void SetUp() override
    {
        m_test_filename =
            "test_logfile_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".log";

        while (logcoe::is_initialized())
        {
            logcoe::shutdown();
        }
    }

    void TearDown() override
    {
        while (logcoe::is_initialized())
        {
            logcoe::shutdown();
        }

        if (std::filesystem::exists(m_test_filename)) std::filesystem::remove(m_test_filename);
    }

    std::string read_log_file(const std::string &filename)
    {
        std::ifstream file(filename);
        if (!file.is_open()) return "";

        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    bool matches_log_pattern(const std::string &text,
                             logcoe::log_level level,
                             const std::string &message,
                             const std::string &source = "")
    {
        std::string level_str;
        switch (level)
        {
            case logcoe::log_level::debug:
                level_str = "DEBUG";
                break;
            case logcoe::log_level::info:
                level_str = "INFO";
                break;
            case logcoe::log_level::warning:
                level_str = "WARNING";
                break;
            case logcoe::log_level::error:
                level_str = "ERROR";
                break;
            case logcoe::log_level::none:
                level_str = "NONE";
                break;
        }

        std::string pattern = "\\[.*?\\] \\[" + level_str + "\\]";

        if (!source.empty()) pattern += " \\[" + source + "\\]";

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
    logcoe::initialize(logcoe::log_level::info, "", true, true, m_test_filename);

    EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::info);

    logcoe::info("Test debug message");

    EXPECT_TRUE(std::filesystem::exists(m_test_filename));

    std::string file_content = read_log_file(m_test_filename);
    EXPECT_TRUE(matches_log_pattern(file_content, logcoe::log_level::info, "Test debug message"));
}

TEST_F(LogcoeTest, LogLevelFiltering)
{
    logcoe::initialize(logcoe::log_level::warning);
    logcoe::set_console_output(m_test_stream);

    logcoe::debug("Debug message");
    logcoe::info("Info message");

    logcoe::warning("Warning message");
    logcoe::error("Error message");

    std::string output = m_test_stream.str();

    EXPECT_FALSE(matches_log_pattern(output, logcoe::log_level::debug, "Debug message"));
    EXPECT_FALSE(matches_log_pattern(output, logcoe::log_level::info, "Info message"));

    EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::warning, "Warning message"));
    EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::error, "Error message"));
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
    logcoe::set_console_output(m_test_stream);

    logcoe::info("Test message");

    std::string output = m_test_stream.str();
    EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::info, "Test message"));
}

TEST_F(LogcoeTest, FileOutput)
{
    logcoe::initialize();

    EXPECT_TRUE(logcoe::set_file_output(m_test_filename).has_value());

    logcoe::info("File test message");

    std::string file_content = read_log_file(m_test_filename);
    EXPECT_TRUE(matches_log_pattern(file_content, logcoe::log_level::info, "File test message"));

    logcoe::disable_file_output();
    logcoe::info("This shouldn't be in the file");

    file_content = read_log_file(m_test_filename);
    EXPECT_FALSE(matches_log_pattern(file_content, logcoe::log_level::info, "This shouldn't be in the file"));
}

TEST_F(LogcoeTest, DisableConsole)
{
    logcoe::initialize();
    logcoe::set_console_output(m_test_stream);

    logcoe::info("Before disable");

    logcoe::disable_console_output();
    logcoe::info("After disable");

    std::string output = m_test_stream.str();
    EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::info, "Before disable"));
    EXPECT_FALSE(matches_log_pattern(output, logcoe::log_level::info, "After disable"));
}

TEST_F(LogcoeTest, SourceField)
{
    logcoe::initialize();
    logcoe::set_console_output(m_test_stream);

    logcoe::info("Message with source", "TestSource");

    std::string output = m_test_stream.str();
    EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::info, "Message with source", "TestSource"));
}

TEST_F(LogcoeTest, TimeFormat)
{
    logcoe::initialize();
    logcoe::set_console_output(m_test_stream);

    [[maybe_unused]] const auto result = logcoe::set_time_format("%H:%M:%S");

    logcoe::info("Custom time format");

    std::string output = m_test_stream.str();
    std::regex time_pattern("\\[\\d{2}:\\d{2}:\\d{2}\\]");
    EXPECT_TRUE(std::regex_search(output, time_pattern));
}
