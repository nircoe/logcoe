#include <logcoe.hpp>
#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

class LogcoeStripTest : public ::testing::Test
{
protected:
    std::stringstream testStream;
    std::string testFilename;

    void SetUp() override
    {
        testFilename =
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
        if (std::filesystem::exists(testFilename)) std::filesystem::remove(testFilename);
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
    logcoe::set_console_output(testStream);

    {
        logcoe::debug("Debug message");
        logcoe::info("Info message");
        logcoe::warning("Warning message");
        logcoe::error("Error message");

        EXPECT_TRUE(testStream.str().empty());
    }

    {
        logcoe::disable_console_output();
        EXPECT_TRUE(testStream.str().empty());
    }
}

TEST_F(LogcoeStripTest, FileOutput)
{
    logcoe::initialize();

    {
        auto result = logcoe::set_file_output(testFilename);
        EXPECT_TRUE(result.has_value());
    }

    {
        EXPECT_FALSE(std::filesystem::exists(testFilename));
    }

    {
        logcoe::disable_file_output();
        EXPECT_FALSE(std::filesystem::exists(testFilename));
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
        logcoe::initialize(logcoe::log_level::debug, "", true, true, testFilename);
        logcoe::shutdown();

        EXPECT_FALSE(std::filesystem::exists(testFilename));
    }
}
