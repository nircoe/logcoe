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
- Use 4 spaces for indentation (no tabs)
- Follow existing naming conventions:
  - `snake_case` for functions and variables
  - `PascalCase` for classes and enums
  - `s_` prefix for static members
- Keep lines under 120 characters
- Add comments for complex logic
- Use `const` and `constexpr` where appropriate

### Example Code Style
```cpp
// Good
namespace logcoe {
    void setLogLevel(LogLevel level) {
        std::lock_guard<std::mutex> lock(s_mutex);
        s_logLevel = level;
    }
}

// Avoid
namespace logcoe{
void setLogLevel(LogLevel level){
std::lock_guard<std::mutex> lock(s_mutex);
s_logLevel=level;
}
```

### Testing Guidelines
- Add tests for new features in appropriate test files
- Ensure thread safety tests pass for concurrent operations
- Test edge cases and error conditions
- Verify cross-platform behavior
- Include performance considerations for new features

### Pull Request Process

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Make your changes following the code style
4. Add/update tests as needed
5. Run tests locally and ensure they pass
6. Commit with clear, descriptive messages
7. Push to your fork (`git push origin feature/amazing-feature`)
8. Open a Pull Request from your fork to the main repository

### Commit Messages
- Use present tense ("Add feature" not "Added feature")
- Keep first line under 50 characters
- Include detailed description if needed
- Reference issues if applicable (`Fixes #123`)

Example:
```
Add custom time format validation

- Validate strftime format strings before applying
- Return error for invalid formats
- Add tests for format validation
- Fixes #45
```

## Platform-Specific Considerations

### Windows
- Test with both MSVC and MinGW compilers
- Verify `localtime_s` usage for thread safety
- Check file path handling with different separators
- Test with different Windows versions if possible

### Linux
- Test with GCC and Clang
- Verify `localtime_r` usage
- Check behavior with different distributions
- Test with different filesystem types

### macOS
- Test with Apple Clang
- Verify compatibility across macOS versions
- Check behavior with case-sensitive/insensitive filesystems

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

### Example: Adding a New Feature

```cpp
// 1. Add to public API (logcoe.hpp)
void setMaxFileSize(size_t maxBytes);

// 2. Add to LoggerImpl (logcoe.cpp)
void LoggerImpl::setMaxFileSize(size_t maxBytes) {
    std::lock_guard<std::mutex> lock(s_mutex);
    s_maxFileSize = maxBytes;
    // Implementation details...
}

// 3. Add wrapper function
namespace logcoe {
    void setMaxFileSize(size_t maxBytes) {
        LoggerImpl::setMaxFileSize(maxBytes);
    }
}

// 4. Add tests
TEST_F(LogcoeTest, MaxFileSizeRotation) {
    logcoe::initialize();
    logcoe::setMaxFileSize(1024);
    // Test implementation...
}
```

## Thread Safety Guidelines

When modifying logcoe:

1. **Always Use Mutex**: Every function that accesses static state must lock `s_mutex`
2. **Minimize Lock Duration**: Perform I/O operations efficiently under lock
3. **Avoid Nested Locks**: Current design uses single mutex to prevent deadlocks
4. **Test Concurrency**: Add thread safety tests for new features

### Thread Safety Checklist
- [ ] Function acquires `s_mutex` before accessing static state
- [ ] Lock scope covers all state modifications
- [ ] No function calls while holding lock that could block indefinitely
- [ ] Thread safety test added for new functionality

## Debugging Tips

### Common Issues
- **Build Failures**: Check C++17 compiler compatibility
- **Test Failures**: Verify file permissions and disk space
- **Thread Issues**: Use thread sanitizer when available
- **Cross-Platform**: Test path separators and line endings

### Debugging Tools
```bash
# Build with debug symbols
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Run with thread sanitizer (GCC/Clang)
cmake -DCMAKE_CXX_FLAGS="-fsanitize=thread" ..

# Run with address sanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" ..
```

### Debugging Logging Issues
- Use different log levels to isolate problems
- Check file permissions for file output issues
- Verify stream redirection for console output problems
- Test with single-threaded execution to isolate thread issues

## Performance Considerations

When contributing:

- **Minimize Lock Contention**: Keep critical sections small
- **Avoid Dynamic Allocation**: Use static storage where possible
- **Efficient String Handling**: Consider string view usage for future optimizations
- **I/O Efficiency**: Batch operations when possible

## Documentation Standards

- Update README.md for user-facing changes
- Add inline comments for complex algorithms
- Update architecture documentation for internal changes
- Include examples for new API functions
- Maintain consistent documentation style

## Questions?

Feel free to open an issue for:
- Bug reports with reproduction steps
- Feature requests with use cases
- Questions about the codebase
- Discussion about implementation approaches
- Help with development setup

We're here to help make contributing to logcoe as smooth as possible!