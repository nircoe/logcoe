#include <logcoe.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <string>

class LogcoeErrorHandlingTests : public ::testing::Test
{
protected:
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

TEST_F(LogcoeErrorHandlingTests, SetFileOutputSuccess)
{
    logcoe::initialize();

    auto result = logcoe::set_file_output(m_test_filename);

    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(std::filesystem::exists(m_test_filename));
}

TEST_F(LogcoeErrorHandlingTests, SetFileOutputFailure)
{
    logcoe::initialize();

    std::string bad_path = "nonexistent_dir/" + m_test_filename;
    auto result = logcoe::set_file_output(bad_path);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), logcoe::error_reason::file_open_failure);
    EXPECT_FALSE(std::filesystem::exists(bad_path));
}

TEST_F(LogcoeErrorHandlingTests, SetFileOutputNotInitialized)
{
    auto result = logcoe::set_file_output(m_test_filename);

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), logcoe::error_reason::not_initialized);
    EXPECT_FALSE(std::filesystem::exists(m_test_filename));
}

TEST_F(LogcoeErrorHandlingTests, SetTimeFormatSuccess)
{
    logcoe::initialize();

    auto result = logcoe::set_time_format("%H:%M:%S");

    EXPECT_TRUE(result.has_value());
}

TEST_F(LogcoeErrorHandlingTests, SetTimeFormatFailure)
{
    logcoe::initialize();

    // strftime copies chars with no % straight through, so something like "bogus" would
    // actually succeed. the only portable way to make it fail is to overflow the 256 byte buffer.
    auto result = logcoe::set_time_format(std::string(300, 'x'));

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), logcoe::error_reason::invalid_time_format);
}

TEST_F(LogcoeErrorHandlingTests, SetTimeFormatNotInitialized)
{
    auto result = logcoe::set_time_format("%H:%M:%S");

    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), logcoe::error_reason::not_initialized);
}
