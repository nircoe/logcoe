#include <logcoe.hpp>
#include <gtest/gtest.h>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class LogcoeThreadTest : public ::testing::Test
{
protected:
    std::string m_test_filename;
    std::stringstream m_test_stream;
    const int NUM_THREADS = 10;
    const int MESSAGES_PER_THREAD = 100;

    void SetUp() override
    {
        m_test_filename =
            "thread_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".log";

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

    int count_log_entries(const std::string &filename, const std::string &pattern)
    {
        std::ifstream file(filename);
        if (!file.is_open()) return 0;

        std::string line;
        int count = 0;

        while (std::getline(file, line))
        {
            if (line.find(pattern) != std::string::npos) count++;
        }

        return count;
    }

    int count_log_entries_in_string(const std::string &content, const std::string &pattern)
    {
        std::istringstream stream(content);
        std::string line;
        int count = 0;

        while (std::getline(stream, line))
        {
            if (line.find(pattern) != std::string::npos) count++;
        }

        return count;
    }
};

TEST_F(LogcoeThreadTest, ConcurrentFileLogging)
{
    logcoe::initialize(logcoe::log_level::debug, "", false, true, m_test_filename);

    std::vector<std::thread> threads;
    std::atomic<int> thread_id_counter(0);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back([this, &thread_id_counter]()
        {
            int thread_id = thread_id_counter++;
            std::string thread_name = "Thread-" + std::to_string(thread_id);

            for (int j = 0; j < MESSAGES_PER_THREAD; j++)
            {
                std::string message = "Message " + std::to_string(j);
                logcoe::info(message, thread_name);

                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }

    for (auto &thread : threads)
        thread.join();

    logcoe::flush();
    logcoe::shutdown();

    int expected_total = NUM_THREADS * MESSAGES_PER_THREAD;

    int actual_total = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        std::string thread_name = "Thread-" + std::to_string(i);
        int count = count_log_entries(m_test_filename, thread_name);

        EXPECT_EQ(count, MESSAGES_PER_THREAD)
            << "Thread " << i << " logged " << count << " messages, expected " << MESSAGES_PER_THREAD;

        actual_total += count;
    }

    EXPECT_EQ(actual_total, expected_total) << "Total messages: " << actual_total << ", expected: " << expected_total;
}

TEST_F(LogcoeThreadTest, ConcurrentConsoleOutput)
{
    logcoe::initialize(logcoe::log_level::debug, "", true, false);
    logcoe::set_console_output(m_test_stream);

    std::vector<std::thread> threads;
    std::atomic<int> thread_id_counter(0);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back([this, &thread_id_counter]()
        {
            int thread_id = thread_id_counter++;
            std::string thread_name = "Thread-" + std::to_string(thread_id);

            for (int j = 0; j < MESSAGES_PER_THREAD; j++)
            {
                std::string message = "Message " + std::to_string(j);
                logcoe::info(message, thread_name);

                std::this_thread::sleep_for(std::chrono::microseconds(10));
            }
        });
    }

    for (auto &thread : threads)
        thread.join();

    std::string output = m_test_stream.str();

    int actual_total = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        std::string thread_name = "Thread-" + std::to_string(i);
        int count = count_log_entries_in_string(output, thread_name);

        EXPECT_EQ(count, MESSAGES_PER_THREAD)
            << "Thread " << i << " logged " << count << " messages, expected " << MESSAGES_PER_THREAD;

        actual_total += count;
    }

    int expected_total = NUM_THREADS * MESSAGES_PER_THREAD;
    EXPECT_EQ(actual_total, expected_total) << "Total messages: " << actual_total << ", expected: " << expected_total;
}

TEST_F(LogcoeThreadTest, ConcurrentLogLevelChange)
{
    logcoe::initialize(logcoe::log_level::info, "", false, true, m_test_filename);

    std::vector<std::thread> threads;
    std::atomic<bool> start_flag(false);

    threads.emplace_back([&start_flag]()
    {
        while (!start_flag.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        for (int i = 0; i < 5; i++)
        {
            logcoe::set_log_level(logcoe::log_level::debug);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            logcoe::set_log_level(logcoe::log_level::warning);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            logcoe::set_log_level(logcoe::log_level::info);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });

    for (int level = 0; level < 3; level++)
    {
        threads.emplace_back([level, &start_flag]()
        {
            while (!start_flag.load())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }

            std::string level_name;
            std::function<void(const std::string &, const std::string &, bool)> log_func;

            switch (level)
            {
                case 0:
                    level_name = "DEBUG";
                    log_func = logcoe::debug;
                    break;
                case 1:
                    level_name = "INFO";
                    log_func = logcoe::info;
                    break;
                case 2:
                    level_name = "WARNING";
                    log_func = logcoe::warning;
                    break;
                default:
                    level_name = "ERROR";
                    log_func = logcoe::error;
                    break;
            }

            for (int i = 0; i < 100; i++)
            {
                std::string message = "Test " + std::to_string(i);
                log_func(message, "", true);
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
            }
        });
    }

    start_flag.store(true);

    for (auto &thread : threads)
        thread.join();

    EXPECT_TRUE(std::filesystem::exists(m_test_filename));
    EXPECT_GT(std::filesystem::file_size(m_test_filename), 0);
}

TEST_F(LogcoeThreadTest, ConcurrentOutputConfigChange)
{
    logcoe::initialize();

    std::vector<std::thread> threads;
    std::atomic<bool> start_flag(false);

    threads.emplace_back([this, &start_flag]()
    {
        while (!start_flag.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        for (int i = 0; i < 5; i++)
        {
            [[maybe_unused]] const auto result = logcoe::set_file_output(m_test_filename);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            logcoe::disable_file_output();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            std::stringstream temp;
            logcoe::set_console_output(temp);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));

            logcoe::set_console_output(std::cout);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }
    });

    threads.emplace_back([&start_flag]()
    {
        while (!start_flag.load())
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }

        for (int i = 0; i < 200; i++)
        {
            logcoe::info("Test message " + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
    });

    start_flag.store(true);

    for (auto &thread : threads)
        thread.join();
}
