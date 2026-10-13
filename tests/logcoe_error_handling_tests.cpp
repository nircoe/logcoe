#include <logcoe.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <string>

//==============================================================================
//                    LogcoeErrorHandlingTests - Setter error results
//==============================================================================

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

//==============================================================================
//                        Set File Output
//==============================================================================

TEST_F(LogcoeErrorHandlingTests, SetFileOutput)
{
    // Test 1: success creates the file
    {
        logcoe::initialize();

        auto result = logcoe::set_file_output(m_test_filename);

        EXPECT_TRUE(result.has_value());
        EXPECT_TRUE(std::filesystem::exists(m_test_filename));

        logcoe::shutdown();
        std::filesystem::remove(m_test_filename);
    }

    // Test 2: a path in a missing directory fails
    {
        logcoe::initialize();

        std::string bad_path = "nonexistent_dir/" + m_test_filename;
        auto result = logcoe::set_file_output(bad_path);

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), logcoe::error_reason::file_open_failure);
        EXPECT_FALSE(std::filesystem::exists(bad_path));

        logcoe::shutdown();
    }

    // Test 3: not initialized
    {
        auto result = logcoe::set_file_output(m_test_filename);

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), logcoe::error_reason::not_initialized);
        EXPECT_FALSE(std::filesystem::exists(m_test_filename));
    }
}

//==============================================================================
//                        Set Time Format
//==============================================================================

TEST_F(LogcoeErrorHandlingTests, SetTimeFormat)
{
    // Test 1: success
    {
        logcoe::initialize();

        auto result = logcoe::set_time_format("%H:%M:%S");

        EXPECT_TRUE(result.has_value());

        logcoe::shutdown();
    }

    // Test 2: a format that overflows the buffer fails
    {
        logcoe::initialize();

        // strftime copies chars with no % straight through, so something like "bogus" would
        // actually succeed. the only portable way to make it fail is to overflow the 256 byte buffer.
        auto result = logcoe::set_time_format(std::string(300, 'x'));

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), logcoe::error_reason::invalid_time_format);

        logcoe::shutdown();
    }

    // Test 3: not initialized
    {
        auto result = logcoe::set_time_format("%H:%M:%S");

        EXPECT_FALSE(result.has_value());
        EXPECT_EQ(result.error(), logcoe::error_reason::not_initialized);
    }
}
