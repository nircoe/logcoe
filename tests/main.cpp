#include <testcoe.hpp>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>

namespace
{
    void print_help()
    {
        std::cout << "Usage: ./logcoe_tests [options]\n";
        std::cout << "       ./logcoe_strip_tests [options]\n";
        std::cout << "Options:\n";
        std::cout << "  --help            Display this help message\n";
        std::cout << "  --all             Run all tests (default)\n";
        std::cout << "  --suite=NAME      Run only the specified test suite\n";
        std::cout << "  --test=SUITE.TEST Run only the specified test\n";
        std::cout << "\n";
        std::cout << "Available test suites:\n";
        std::cout << "  LogcoeTests              - Functional tests\n";
        std::cout << "  LogcoeThreadTests        - Thread safety tests\n";
        std::cout << "  LogcoeErrorHandlingTests - std::expected results of the setters\n";
        std::cout << "  LogcoeStripTests         - NDEBUG stub tests\n";
        std::cout << "\n";
        std::cout << "LogcoeStripTests exists only in logcoe_strip_tests, the other suites only in logcoe_tests.\n";
        std::cout << "\n";
        std::cout << "Example usage:\n";
        std::cout << "  ./logcoe_tests --suite=LogcoeThreadTests\n";
        std::cout << "  ./logcoe_tests --test=LogcoeThreadTests.ConcurrentFileLogging\n";
    }
} // namespace

int main(int argc, char **argv)
{
    std::cout << "====================================================\n";
    std::cout << "                 logcoe Test Suite                  \n";
    std::cout << "====================================================\n";
    std::cout << "\n";
    std::cout << "Testing logcoe: functional behavior, thread safety, std::expected error handling, NDEBUG stubs\n";
    std::cout << "\n";

    constexpr std::string_view SUITE_PREFIX = "--suite=";
    constexpr std::string_view TEST_PREFIX = "--test=";

    bool ask_for_all = false;
    std::string suite_name;
    std::string test_name;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "--help")
        {
            print_help();
            return 0;
        }
        else if (arg == "--all")
            ask_for_all = true;
        else if (!ask_for_all && arg.starts_with(SUITE_PREFIX))
        {
            suite_name = arg.substr(SUITE_PREFIX.size());
            if (suite_name.empty())
            {
                std::cerr << "Error: --suite= requires a suite name\n";
                return 1;
            }
        }
        else if (!ask_for_all && arg.starts_with(TEST_PREFIX))
        {
            std::string full_test = arg.substr(TEST_PREFIX.size());
            std::size_t dot_pos = full_test.find('.');
            if (dot_pos == std::string::npos || dot_pos == 0 || dot_pos + 1 == full_test.size())
            {
                std::cerr << "Error: --test= requires SUITE.TEST\n";
                return 1;
            }
            suite_name = full_test.substr(0, dot_pos);
            test_name = full_test.substr(dot_pos + 1);
        }
    }

    testcoe::init(&argc, argv);

    if (ask_for_all || (test_name.empty() && suite_name.empty()))
    {
        std::cout << "Running all logcoe tests...\n";
        return testcoe::run();
    }

    if (!test_name.empty())
    {
        std::cout << "Running test: " << suite_name << "." << test_name << "\n";
        return testcoe::run_test(suite_name, test_name);
    }

    std::cout << "Running suite: " << suite_name << "\n";
    return testcoe::run_suite(suite_name);
}
