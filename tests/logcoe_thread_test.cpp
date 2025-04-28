#include <gtest/gtest.h>
#include <logcoe.hpp>
#include <thread>
#include <vector>
#include <atomic>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <filesystem>

class LogcoeThreadTest : public ::testing::Test
{
protected:
    std::string testFilename;
    std::stringstream testStream;
    const int NUM_THREADS = 10;
    const int MESSAGES_PER_THREAD = 100;

    void SetUp() override
    {
        testFilename = "thread_test_" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count()) + ".log";

        logcoe::shutdown();
    }

    void TearDown() override
    {
        logcoe::shutdown();

        if (std::filesystem::exists(testFilename))
            std::filesystem::remove(testFilename);
    }

    int countLogEntries(const std::string &filename, const std::string &pattern)
    {
        std::ifstream file(filename);
        if (!file.is_open())
            return 0;

        std::string line;
        int count = 0;

        while (std::getline(file, line))
        {
            if (line.find(pattern) != std::string::npos)
                count++;
        }

        return count;
    }

    int countLogEntriesInString(const std::string &content, const std::string &pattern)
    {
        std::istringstream stream(content);
        std::string line;
        int count = 0;

        while (std::getline(stream, line))
        {
            if (line.find(pattern) != std::string::npos)
                count++;
        }

        return count;
    }
};

TEST_F(LogcoeThreadTest, ConcurrentFileLogging)
{
    logcoe::initialize(logcoe::LogLevel::DEBUG, "", false, true, testFilename);

    std::vector<std::thread> threads;
    std::atomic<int> thread_id_counter(0);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back([this, &thread_id_counter]()
                             {
            int thread_id = thread_id_counter++;
            std::string thread_name = "Thread-" + std::to_string(thread_id);
            
            for (int j = 0; j < MESSAGES_PER_THREAD; j++) {
                std::string message = "Message " + std::to_string(j);
                logcoe::info(message, thread_name);
                
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            } });
    }

    for (auto &thread : threads)
        thread.join();

    logcoe::flush();
    logcoe::shutdown();

    int expectedTotal = NUM_THREADS * MESSAGES_PER_THREAD;

    int actualTotal = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        std::string thread_name = "Thread-" + std::to_string(i);
        int count = countLogEntries(testFilename, thread_name);

        EXPECT_EQ(count, MESSAGES_PER_THREAD)
            << "Thread " << i << " logged " << count << " messages, expected " << MESSAGES_PER_THREAD;

        actualTotal += count;
    }

    EXPECT_EQ(actualTotal, expectedTotal)
        << "Total messages: " << actualTotal << ", expected: " << expectedTotal;
}

TEST_F(LogcoeThreadTest, ConcurrentConsoleOutput)
{
    logcoe::initialize(logcoe::LogLevel::DEBUG, "", true, false);
    logcoe::setConsoleOutput(testStream);

    std::vector<std::thread> threads;
    std::atomic<int> thread_id_counter(0);

    for (int i = 0; i < NUM_THREADS; i++)
    {
        threads.emplace_back([this, &thread_id_counter]()
        {
            int thread_id = thread_id_counter++;
            std::string thread_name = "Thread-" + std::to_string(thread_id);
            
            for (int j = 0; j < MESSAGES_PER_THREAD; j++) {
                std::string message = "Message " + std::to_string(j);
                logcoe::info(message, thread_name);
                
                std::this_thread::sleep_for(std::chrono::microseconds(10));
            } 
        });
    }

    for (auto &thread : threads)
        thread.join();

    std::string output = testStream.str();

    int actualTotal = 0;
    for (int i = 0; i < NUM_THREADS; i++)
    {
        std::string thread_name = "Thread-" + std::to_string(i);
        int count = countLogEntriesInString(output, thread_name);

        EXPECT_EQ(count, MESSAGES_PER_THREAD)
            << "Thread " << i << " logged " << count << " messages, expected " << MESSAGES_PER_THREAD;

        actualTotal += count;
    }

    int expectedTotal = NUM_THREADS * MESSAGES_PER_THREAD;
    EXPECT_EQ(actualTotal, expectedTotal)
        << "Total messages: " << actualTotal << ", expected: " << expectedTotal;
}

TEST_F(LogcoeThreadTest, ConcurrentLogLevelChange)
{
    logcoe::initialize(logcoe::LogLevel::INFO, "", false, true, testFilename);

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
            logcoe::setLogLevel(logcoe::LogLevel::DEBUG);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            logcoe::setLogLevel(logcoe::LogLevel::WARNING);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            logcoe::setLogLevel(logcoe::LogLevel::INFO);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        } 
    });

    for (int level = 0; level < 3; level++)
    {
        threads.emplace_back([this, level, &start_flag]()
        {
            while (!start_flag.load()) 
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            
            std::string level_name;
            std::function<void(const std::string&, const std::string&, bool)> log_func;
            
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

    EXPECT_TRUE(std::filesystem::exists(testFilename));
    EXPECT_GT(std::filesystem::file_size(testFilename), 0);
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
            logcoe::setFileOutput(testFilename);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            
            logcoe::disableFileOutput();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            
            std::stringstream temp;
            logcoe::setConsoleOutput(temp);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            
            logcoe::setConsoleOutput(std::cout);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        } 
    });

    threads.emplace_back([this, &start_flag]()
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