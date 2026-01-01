# Contributing to logcoe

Thank you for your interest in contributing to logcoe!

## Development Setup

### Prerequisites
- CMake 3.14+
- C++17 compatible compiler
- Git

### Building from Source

```bash
git clone https://github.com/nircoe/logcoe.git
cd logcoe
mkdir build && cd build
cmake -DLOGCOE_BUILD_TESTS=ON ..
cmake --build .
```

### Running Tests

logcoe uses [testcoe](https://github.com/nircoe/testcoe) for its test suite, providing enhanced test visualization:

```bash
# Run all tests
./tests/logcoe_tests

# Tests include:
# - Basic functionality tests
# - Thread safety verification
# - Cross-platform compatibility
# - Performance validation
```

## Project Structure

```
logcoe/
├── include/
│   └── logcoe.hpp          # Public API header
├── src/
│   └── logcoe.cpp          # Implementation
├── tests/
│   ├── main.cpp            # Test runner
│   ├── logcoe_test.cpp     # Functional tests
│   └── logcoe_thread_test.cpp # Thread safety tests
├── docs/                   # Documentation
└── .github/workflows/      # CI configuration
```

## Continuous Integration

All pull requests are automatically tested on:

- **Windows**: MSVC 2019/2022 and MinGW
- **Linux**: GCC 7+ and Clang 5+  
- **macOS**: Apple Clang (Xcode 10+)

### CI Pipeline Details

The CI runs the following checks:
1. Build the library with different compilers
2. Run comprehensive test suite
3. Verify thread safety under load
4. Test cross-platform compatibility
5. Check memory safety (when available)

## Making Changes

### Code Style
- Follow existing naming conventions:
  - `camelCase` for functions and variables
  - `PascalCase` for classes and enums
  - `s_` prefix for static members
- Keep lines under 120 characters
- Add comments for complex logic
- Use `const` and `constexpr` where appropriate

### Example Code Style
```cpp
namespace logcoe 
{
    void setLogLevel(LogLevel level) 
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_logLevel = level;
    }
}
```

### Testing Guidelines
- Add tests for new features in appropriate test files
- Ensure thread safety tests pass for concurrent operations
- Test edge cases and error conditions
- Verify cross-platform behavior through CI
- Include performance considerations for new features

### Pull Request Process

1. Fork the repository
2. Create a feature branch
3. Make your changes following the code style
4. Add/update tests as needed
5. Run tests locally and ensure they pass
6. Commit with clear, descriptive messages
7. Push to your fork
8. Open a Pull Request from your fork to the main repository

### Commit Messages
- Use prefix for PR title `[Subject]: <PR title>`
- PR description should describe the major changes in bullet-points
- Sqaushed commit title should be the PR title, and the message should be PR description

Example:
```
[Time Format]: Add custom time format validation

- Validate strftime format strings before applying
- Return error for invalid formats
- Add tests for format validation
```

## Adding New Features

When adding features:

1. **Update Public API** (if needed):
   - Modify `include/logcoe.hpp`
   - Maintain backward compatibility
   - Add documentation comments

2. **Implement in LoggerImpl**:
   - Add to `src/logcoe.cpp`
   - Ensure thread safety with proper locking
   - Handle error cases gracefully

3. **Add Tests**:
   - Functional tests in `logcoe_test.cpp`
   - Thread safety tests in `logcoe_thread_test.cpp`
   - Test both success and failure cases

4. **Update Documentation**:
   - Update README.md examples
   - Add to API reference section
   - Update architecture docs if needed

## Thread Safety Guidelines

When modifying logcoe:

1. **Always Use Mutex**: Every function that accesses static state must lock `s_mutex`
2. **Minimize Lock Duration**: Perform I/O operations efficiently under lock
3. **Avoid Nested Locks**: Current design uses single mutex to prevent deadlocks
4. **Test Concurrency**: Add thread safety tests for new features

## Questions?

Feel free to reach out at nircoe@gmail.com

I'm here to help make contributing to logcoe as smooth as possible!