#include <gtest/gtest.h>
#include <logcoe.hpp>

#include <filesystem>
#include <string>
#include <chrono>

class LogcoeErrorHandlingTest : public ::testing::Test
{
protected:
    std::string testFilename;

    void SetUp() override
    {
        testFilename = "test_logfile_" +
                       std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".log";

        while (logcoe::is_initialized()) { logcoe::shutdown(); }
    }

    void TearDown() override
    {
        while (logcoe::is_initialized()) { logcoe::shutdown(); }

        if (std::filesystem::exists(testFilename))
            std::filesystem::remove(testFilename);
    }
};

TEST_F(LogcoeErrorHandlingTest, SetFileOutputSuccess)
{
    logcoe::initialize();

    auto result = logcoe::set_file_output(testFilename);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(std::filesystem::exists(testFilename));
}

TEST_F(LogcoeErrorHandlingTest, SetFileOutputFailure)
{
    logcoe::initialize();

    std::string badPath = "nonexistent_dir_" +
                          std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + "/" +
                          testFilename;
    auto result = logcoe::set_file_output(badPath);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), logcoe::error_reason::file_open_failure);
    EXPECT_FALSE(std::filesystem::exists(badPath));
}

TEST_F(LogcoeErrorHandlingTest, SetTimeFormatSuccess)
{
    logcoe::initialize();

    auto result = logcoe::set_time_format("%H:%M:%S");

    EXPECT_TRUE(result.has_value());
}

TEST_F(LogcoeErrorHandlingTest, SetTimeFormatFailure)
{
    logcoe::initialize();

    // strftime copies chars with no % straight through, so something like "bogus" would
    // actually succeed. the only portable way to make it fail is to overflow the 256 byte buffer.
    auto result = logcoe::set_time_format(std::string(300, 'x'));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), logcoe::error_reason::invalid_time_format);
}
