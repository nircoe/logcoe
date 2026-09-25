#include <gtest/gtest.h>
#include <logcoe.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <string>

class LogcoeStripTest : public ::testing::Test
{
protected:
    std::stringstream testStream;
    std::string testFilename;

    void SetUp() override
    {
        testFilename = "test_logfile_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".log";
        while(logcoe::isInitialized()) { logcoe::shutdown(); }
    }

    void TearDown() override
    {
        while(logcoe::isInitialized()) { logcoe::shutdown(); }
        if (std::filesystem::exists(testFilename))
            std::filesystem::remove(testFilename);
    }
};

TEST_F(LogcoeStripTest, InitializeAndStateQueries)
{
    logcoe::initialize(logcoe::LogLevel::INFO);

    {
        EXPECT_FALSE(logcoe::isInitialized());
    }

    {
        EXPECT_EQ(logcoe::getLogLevel(), logcoe::LogLevel::NONE);
    }

    {
        logcoe::setLogLevel(logcoe::LogLevel::DEBUG);
        EXPECT_EQ(logcoe::getLogLevel(), logcoe::LogLevel::NONE);
    }
}

TEST_F(LogcoeStripTest, ConsoleOutput)
{
    logcoe::initialize();
    logcoe::setConsoleOutput(testStream);

    {
        logcoe::debug("Debug message");
        logcoe::info("Info message");
        logcoe::warning("Warning message");
        logcoe::error("Error message");

        EXPECT_TRUE(testStream.str().empty());
    }

    {
        logcoe::disableConsoleOutput();
        EXPECT_TRUE(testStream.str().empty());
    }
}

TEST_F(LogcoeStripTest, FileOutput)
{
    logcoe::initialize();

    {
        bool result = logcoe::setFileOutput(testFilename);
        EXPECT_FALSE(result);
    }

    {
        EXPECT_FALSE(std::filesystem::exists(testFilename));
    }

    {
        logcoe::disableFileOutput();
        EXPECT_FALSE(std::filesystem::exists(testFilename));
    }
}

TEST_F(LogcoeStripTest, TimeFormat)
{
    logcoe::initialize();

    {
        EXPECT_NO_THROW(logcoe::setTimeFormat("bogus"));
    }

    {
        EXPECT_NO_THROW(logcoe::setTimeFormat("%H:%M:%S"));
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
        logcoe::initialize(logcoe::LogLevel::DEBUG, "", true, true, testFilename);
        logcoe::shutdown();

        EXPECT_FALSE(std::filesystem::exists(testFilename));
    }
}
