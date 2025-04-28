#include <testcoe.hpp>
#include <iostream>

int main(int argc, char **argv)
{
    // Initialize testcoe (replaces Google Test initialization)
    testcoe::init(&argc, argv);

    // Run all tests
    return testcoe::run();
}