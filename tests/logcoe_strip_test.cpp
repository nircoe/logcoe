#include <logcoe.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <sstream>
#include <string>

class LogcoeStripTest : public ::testing::Test
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
};

TEST_F(LogcoeStripTest, InitializeAndStateQueries)
{
    logcoe::initialize(logcoe::log_level::info);

    {
        EXPECT_FALSE(logcoe::is_initialized());
    }

    {
        EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::none);
    }

    {
        logcoe::set_log_level(logcoe::log_level::debug);
        EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::none);
    }
}

TEST_F(LogcoeStripTest, ConsoleOutput)
{
    logcoe::initialize();
    logcoe::set_console_output(m_test_stream);

    {
        logcoe::debug("Debug message");
        logcoe::info("Info message");
        logcoe::warning("Warning message");
        logcoe::error("Error message");

        EXPECT_TRUE(m_test_stream.str().empty());
    }

    {
        logcoe::disable_console_output();
        EXPECT_TRUE(m_test_stream.str().empty());
    }
}

TEST_F(LogcoeStripTest, FileOutput)
{
    logcoe::initialize();

    {
        auto result = logcoe::set_file_output(m_test_filename);
        EXPECT_TRUE(result.has_value());
    }

    {
        EXPECT_FALSE(std::filesystem::exists(m_test_filename));
    }

    {
        logcoe::disable_file_output();
        EXPECT_FALSE(std::filesystem::exists(m_test_filename));
    }
}

TEST_F(LogcoeStripTest, TimeFormat)
{
    logcoe::initialize();

    {
        auto result = logcoe::set_time_format("bogus");
        EXPECT_TRUE(result.has_value());
    }

    {
        auto result = logcoe::set_time_format("%H:%M:%S");
        EXPECT_TRUE(result.has_value());
    }
}

TEST_F(LogcoeStripTest, Lifecycle)
{
    {
        EXPECT_NO_THROW(logcoe::flush());
    }

    {
        EXPECT_NO_THROW(logcoe::shutdown());
    }

    {
        logcoe::initialize();
        logcoe::shutdown();
        logcoe::initialize(logcoe::log_level::debug, "", true, true, m_test_filename);
        logcoe::shutdown();

        EXPECT_FALSE(std::filesystem::exists(m_test_filename));
    }
}
