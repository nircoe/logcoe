#include <logcoe.hpp>
#include <gtest/gtest.h>
#include <support/counted_value.hpp>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <ostream>
#include <regex>
#include <set>
#include <sstream>
#include <streambuf>
#include <string>
#include <string_view>
#include <system_error>

using namespace test_utils;

//==============================================================================
//                        Test Helpers
//==============================================================================

namespace
{
    class sync_counting_buf : public std::streambuf
    {
        int m_sync_count = 0;

    public:
        int sync_count() const { return m_sync_count; }
        void reset() { m_sync_count = 0; }

    protected:
        int_type overflow(int_type ch) override { return traits_type::not_eof(ch); }
        int sync() override
        {
            ++m_sync_count;
            return 0;
        }
    };

} // namespace

//==============================================================================
//                    LogcoeTests - Functional behavior
//==============================================================================

class LogcoeTests : public ::testing::Test
{
protected:
    std::stringstream m_test_stream;
    std::string m_test_filename;
    std::set<std::string> m_existing_default_logs;

    void SetUp() override
    {
        m_test_filename =
            "test_logfile_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".log";

        while (logcoe::is_initialized())
        {
            logcoe::shutdown();
        }

        m_existing_default_logs = list_default_logs();
    }

    void TearDown() override
    {
        while (logcoe::is_initialized())
        {
            logcoe::shutdown();
        }

        if (std::filesystem::exists(m_test_filename)) std::filesystem::remove(m_test_filename);

        remove_created_default_logs();
    }

    std::set<std::string> list_default_logs()
    {
        std::set<std::string> names;
        std::error_code ec;
        for (const auto &entry : std::filesystem::directory_iterator(".", ec))
        {
            const std::string name = entry.path().filename().string();
            if (name.starts_with("logcoe_")) names.insert(name);
        }
        return names;
    }

    std::set<std::string> created_default_logs()
    {
        std::set<std::string> created;
        for (const auto &name : list_default_logs())
            if (!m_existing_default_logs.contains(name)) created.insert(name);
        return created;
    }

    void remove_created_default_logs()
    {
        std::error_code ec;
        for (const auto &name : created_default_logs())
            std::filesystem::remove_all(name, ec);
    }

    void expect_default_logs_are_plain_files()
    {
        const auto created = created_default_logs();
        EXPECT_FALSE(created.empty());
        const std::regex shape("logcoe_\\d{4}-\\d{2}-\\d{2}_\\d{2}-\\d{2}-\\d{2}\\.log");
        for (const auto &name : created)
        {
            EXPECT_TRUE(std::filesystem::is_regular_file(name)) << name;
            EXPECT_TRUE(std::regex_match(name, shape)) << name;
        }
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

//==============================================================================
//                        Initialization
//==============================================================================

TEST_F(LogcoeTests, Initialization)
{
    // Test 1: default initialization
    {
        logcoe::initialize();

        EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::debug);

        logcoe::info("Test info message");

        logcoe::shutdown();
    }

    // Test 2: custom initialization with a file
    {
        logcoe::initialize(logcoe::log_level::info, "", true, true, m_test_filename);

        EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::info);

        logcoe::info("Test debug message");

        EXPECT_TRUE(std::filesystem::exists(m_test_filename));

        std::string file_content = read_log_file(m_test_filename);
        EXPECT_TRUE(matches_log_pattern(file_content, logcoe::log_level::info, "Test debug message"));

        logcoe::shutdown();
    }

    // Test 3: out of range log level
    {
        logcoe::initialize(static_cast<logcoe::log_level>(7));
        EXPECT_TRUE(logcoe::is_initialized());

        logcoe::set_console_output(m_test_stream);
        logcoe::set_log_level(logcoe::log_level::info);
        logcoe::info("After out of range level");

        EXPECT_TRUE(matches_log_pattern(m_test_stream.str(), logcoe::log_level::info, "After out of range level"));

        logcoe::shutdown();
        EXPECT_FALSE(logcoe::is_initialized());
    }
}

//==============================================================================
//                        Log Level
//==============================================================================

TEST_F(LogcoeTests, LogLevel)
{
    // Test 1: messages below the level are filtered
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

        logcoe::shutdown();
    }

    // Test 2: set_log_level changes the level
    {
        logcoe::initialize(logcoe::log_level::error);
        EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::error);

        logcoe::set_log_level(logcoe::log_level::debug);
        EXPECT_EQ(logcoe::get_log_level(), logcoe::log_level::debug);
    }
}

//==============================================================================
//                        Console
//==============================================================================

TEST_F(LogcoeTests, Console)
{
    // Test 1: output goes to the redirected stream
    {
        logcoe::initialize();
        logcoe::set_console_output(m_test_stream);

        logcoe::info("Test message");

        std::string output = m_test_stream.str();
        EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::info, "Test message"));

        logcoe::shutdown();
    }

    // Test 2: disable_console_output stops console output
    {
        m_test_stream.str("");
        logcoe::initialize();
        logcoe::set_console_output(m_test_stream);

        logcoe::info("Before disable");

        logcoe::disable_console_output();
        logcoe::info("After disable");

        std::string output = m_test_stream.str();
        EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::info, "Before disable"));
        EXPECT_FALSE(matches_log_pattern(output, logcoe::log_level::info, "After disable"));

        logcoe::shutdown();
    }

    // Test 3: source field appears in the message
    {
        m_test_stream.str("");
        logcoe::initialize();
        logcoe::set_console_output(m_test_stream);

        logcoe::info("Message with source", "TestSource");

        std::string output = m_test_stream.str();
        EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::info, "Message with source", "TestSource"));
    }
}

//==============================================================================
//                        String View Parameters
//==============================================================================

TEST_F(LogcoeTests, StringViewParameters)
{
    logcoe::initialize();
    logcoe::set_console_output(m_test_stream);

    // Test 1: message and source are read by their length, not up to a null terminator
    {
        const std::string buffer = "HelloWorld";
        const std::string_view view = buffer;

        logcoe::info(view.substr(0, 5), view.substr(5));

        EXPECT_TRUE(m_test_stream.str().ends_with("[World]: Hello\n"));
    }

    // Test 2: literal and std::string callers log the same text
    {
        m_test_stream.str("");

        logcoe::warning("literal message", "literal_source");
        logcoe::error(std::string("owned message"), std::string("owned_source"));

        std::string output = m_test_stream.str();
        EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::warning, "literal message", "literal_source"));
        EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::error, "owned message", "owned_source"));
    }

    // Test 3: an empty source view falls back to the default source
    {
        logcoe::shutdown();
        m_test_stream.str("");
        logcoe::initialize(logcoe::log_level::debug, "DefaultSource");
        logcoe::set_console_output(m_test_stream);

        logcoe::debug(std::string_view("view message"), std::string_view());

        EXPECT_TRUE(
            matches_log_pattern(m_test_stream.str(), logcoe::log_level::debug, "view message", "DefaultSource"));
    }
}

//==============================================================================
//                        Formatted Logging
//==============================================================================

TEST_F(LogcoeTests, FormattedLogging)
{
    logcoe::initialize();
    logcoe::set_console_output(m_test_stream);

    // Test 1: each f function formats its arguments at its own level
    {
        logcoe::debugf("x = {}", 1);
        logcoe::infof("{} and {}", "a", 2.5);
        logcoe::warningf("no arguments");
        logcoe::errorf("{:>4}|", 7);

        std::string output = m_test_stream.str();
        EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::debug, "x = 1"));
        EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::info, "a and 2.5"));
        EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::warning, "no arguments"));
        EXPECT_TRUE(matches_log_pattern(output, logcoe::log_level::error, "   7\\|"));
    }

    // Test 2: the default source from initialize is used
    {
        logcoe::shutdown();
        m_test_stream.str("");
        logcoe::initialize(logcoe::log_level::debug, "DefaultSource");
        logcoe::set_console_output(m_test_stream);

        logcoe::infof("x = {}", 1);

        EXPECT_TRUE(matches_log_pattern(m_test_stream.str(), logcoe::log_level::info, "x = 1", "DefaultSource"));
    }

    // Test 3: a filtered level skips std::format, an enabled one runs it
    {
        logcoe::set_log_level(logcoe::log_level::warning);
        g_format_count = 0;

        logcoe::debugf("{}", counted_value{});
        logcoe::infof("{}", counted_value{});
        EXPECT_EQ(g_format_count, 0);

        logcoe::warningf("{}", counted_value{});
        logcoe::errorf("{}", counted_value{});
        EXPECT_EQ(g_format_count, 2);
    }
}

//==============================================================================
//                        Flush
//==============================================================================

TEST_F(LogcoeTests, Flush)
{
    // Test 1: flush false does not flush the console
    {
        logcoe::initialize(logcoe::log_level::debug);
        sync_counting_buf buf;
        std::ostream stream(&buf);
        logcoe::set_console_output(stream);
        buf.reset();

        logcoe::debug("m", "", false);
        logcoe::info("m", "", false);
        logcoe::warning("m", "", false);
        logcoe::error("m", "", false);
        EXPECT_EQ(buf.sync_count(), 0);

        logcoe::flush();
        EXPECT_EQ(buf.sync_count(), 1);

        logcoe::disable_console_output();
        logcoe::shutdown();
    }

    // Test 2: flush true flushes the console on every level
    {
        logcoe::initialize(logcoe::log_level::debug);
        sync_counting_buf buf;
        std::ostream stream(&buf);
        logcoe::set_console_output(stream);

        buf.reset();
        logcoe::debug("m", "", true);
        EXPECT_EQ(buf.sync_count(), 1);

        buf.reset();
        logcoe::info("m", "", true);
        EXPECT_EQ(buf.sync_count(), 1);

        buf.reset();
        logcoe::warning("m", "", true);
        EXPECT_EQ(buf.sync_count(), 1);

        buf.reset();
        logcoe::error("m", "", true);
        EXPECT_EQ(buf.sync_count(), 1);

        logcoe::disable_console_output();
    }
}

//==============================================================================
//                        File Output
//==============================================================================

TEST_F(LogcoeTests, FileOutput)
{
    // Test 1: file output can be enabled and disabled
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
}

//==============================================================================
//                        Time Format
//==============================================================================

TEST_F(LogcoeTests, TimeFormat)
{
    // Test 1: custom time format is used in the output
    {
        logcoe::initialize();
        logcoe::set_console_output(m_test_stream);

        [[maybe_unused]] const auto result = logcoe::set_time_format("%H:%M:%S");

        logcoe::info("Custom time format");

        std::string output = m_test_stream.str();
        std::regex time_pattern("\\[\\d{2}:\\d{2}:\\d{2}\\]");
        EXPECT_TRUE(std::regex_search(output, time_pattern));
    }
}

//==============================================================================
//                        Default Filename
//==============================================================================

TEST_F(LogcoeTests, DefaultFilename)
{
    // Test 1: initialize with an empty filename
    {
        logcoe::initialize(logcoe::log_level::debug, "", true, true);

        expect_default_logs_are_plain_files();

        // Close the file before deleting it. A leftover log would let the next block pass without creating its own.
        logcoe::shutdown();
        remove_created_default_logs();
    }

    // Test 2: set_file_output with an empty filename
    {
        logcoe::initialize();

        EXPECT_TRUE(logcoe::set_file_output("").has_value());

        expect_default_logs_are_plain_files();

        logcoe::shutdown();
        remove_created_default_logs();
    }

    // Test 3: default filename ignores the time format
    {
        logcoe::initialize();
        EXPECT_TRUE(logcoe::set_time_format("%H:%M:%S").has_value());

        EXPECT_TRUE(logcoe::set_file_output("").has_value());

        expect_default_logs_are_plain_files();
    }
}
