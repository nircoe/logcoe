#include <logcoe.hpp>
#include <gtest/gtest.h>
#include <chrono>
#include <filesystem>
#include <format>
#include <sstream>
#include <string>

namespace
{
    struct counted_value
    {
    };

    int g_format_count = 0;
} // namespace

template <> struct std::formatter<counted_value>
{
    constexpr auto parse(std::format_parse_context &ctx) { return ctx.begin(); }
    auto format(const counted_value &, std::format_context &ctx) const
    {
        ++g_format_count;
        return std::format_to(ctx.out(), "counted");
    }
};

//==============================================================================
//                    LogcoeStripTests - NDEBUG stubs
//==============================================================================

class LogcoeStripTests : public ::testing::Test
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

//==============================================================================
//                        Initialize And State Queries
//==============================================================================

TEST_F(LogcoeStripTests, InitializeAndStateQueries)
{
    logcoe::initialize(logcoe::log_level::info);

    // Test 1: is_initialized is always false
    {
        EXPECT_FALSE(logcoe::is_initialized());
    }

    // Test 2: log level stays none
    {
        EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::none);
    }

    // Test 3: set_log_level has no effect
    {
        logcoe::set_log_level(logcoe::log_level::debug);
        EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::none);
    }
}

//==============================================================================
//                        Console Output
//==============================================================================

TEST_F(LogcoeStripTests, ConsoleOutput)
{
    logcoe::initialize();
    logcoe::set_console_output(m_test_stream);

    // Test 1: log calls write nothing to the console
    {
        logcoe::debug("Debug message");
        logcoe::info("Info message");
        logcoe::warning("Warning message");
        logcoe::error("Error message");

        EXPECT_TRUE(m_test_stream.str().empty());
    }

    // Test 2: disable_console_output writes nothing
    {
        logcoe::disable_console_output();
        EXPECT_TRUE(m_test_stream.str().empty());
    }

    // Test 3: f functions write nothing and skip std::format
    {
        g_format_count = 0;

        logcoe::debugf("{}", counted_value{});
        logcoe::infof("{}", counted_value{});
        logcoe::warningf("{}", counted_value{});
        logcoe::errorf("{}", counted_value{});

        EXPECT_TRUE(m_test_stream.str().empty());
        EXPECT_EQ(g_format_count, 0);
    }
}

//==============================================================================
//                        File Output
//==============================================================================

TEST_F(LogcoeStripTests, FileOutput)
{
    logcoe::initialize();

    // Test 1: set_file_output succeeds
    {
        auto result = logcoe::set_file_output(m_test_filename);
        EXPECT_TRUE(result.has_value());
    }

    // Test 2: no file is created
    {
        EXPECT_FALSE(std::filesystem::exists(m_test_filename));
    }

    // Test 3: disable_file_output creates no file
    {
        logcoe::disable_file_output();
        EXPECT_FALSE(std::filesystem::exists(m_test_filename));
    }
}

//==============================================================================
//                        Time Format
//==============================================================================

TEST_F(LogcoeStripTests, TimeFormat)
{
    logcoe::initialize();

    // Test 1: any time format is accepted
    {
        auto result = logcoe::set_time_format("bogus");
        EXPECT_TRUE(result.has_value());
    }

    // Test 2: a valid time format is accepted
    {
        auto result = logcoe::set_time_format("%H:%M:%S");
        EXPECT_TRUE(result.has_value());
    }
}

//==============================================================================
//                        Lifecycle
//==============================================================================

TEST_F(LogcoeStripTests, Lifecycle)
{
    // Test 1: flush doesn't throw
    {
        EXPECT_NO_THROW(logcoe::flush());
    }

    // Test 2: shutdown without initialize doesn't throw
    {
        EXPECT_NO_THROW(logcoe::shutdown());
    }

    // Test 3: initialize with file output creates no file
    {
        logcoe::initialize();
        logcoe::shutdown();
        logcoe::initialize(logcoe::log_level::debug, "", true, true, m_test_filename);
        logcoe::shutdown();

        EXPECT_FALSE(std::filesystem::exists(m_test_filename));
    }
}
