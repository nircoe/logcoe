# Contributing to logcoe

## Development Setup

### Prerequisites
- CMake 3.22+
- C++23 compiler with `<expected>` (GCC 12+, Clang 16+, Xcode 15+, MSVC 2022 17.3+)
- Git, CMake fetches testcoe from GitHub when it is not installed

### Building from Source

```bash
git clone https://github.com/nircoe/logcoe.git
cd logcoe
mkdir build && cd build
cmake -DLOGCOE_BUILD_TESTS=ON ..
cmake --build .
```

### Running Tests

Tests use [testcoe](https://github.com/nircoe/testcoe), a GoogleTest wrapper.
There are two executables:

- `logcoe_tests` covers the real implementation, build without `NDEBUG` (Debug)
- `logcoe_strip_tests` covers the no-op stubs, build with `NDEBUG` (Release)

```bash
# Debug build
./tests/logcoe_tests

# Release build
./tests/logcoe_strip_tests
```

## Project Structure

```
logcoe/
├── cmake/
│   └── utils.cmake                    # Warning flags, MinGW static runtime, testcoe warning suppression
├── include/
│   └── logcoe.hpp                     # Public API header
├── src/
│   └── logcoe.cpp                     # Implementation
├── tests/
│   ├── CMakeLists.txt
│   ├── main.cpp                       # Test runner
│   ├── logcoe_test.cpp                # Functional tests
│   ├── logcoe_thread_test.cpp         # Thread safety tests
│   ├── logcoe_error_handling_test.cpp # std::expected results of the setters
│   └── logcoe_strip_test.cpp          # NDEBUG stub tests
├── docs/                              # Documentation
└── .github/workflows/                 # CI configuration
```

## Continuous Integration

GitHub Actions runs on pushes to `main` and on pull requests to `main`, skipping draft PRs.
Each job configures with `-DLOGCOE_BUILD_TESTS=ON`, builds, then runs one test executable.
Compilers are the ones on the GitHub runners, versions are not pinned.

- Windows: MSVC Debug and Release, MinGW (GCC) Debug
- Linux: GCC Debug and Release, Clang Debug
- macOS: Apple Clang Debug and Release

Debug jobs run `logcoe_tests`, Release jobs run `logcoe_strip_tests`.
CI does not run sanitizers or benchmarks.

## Making Changes

### Code Style
- `snake_case` for functions, variables, types, and enumerators
- `g_` prefix for anonymous-namespace variables
- Trailing underscore for a parameter that would shadow another identifier in scope
  (a `flush` parameter becomes `flush_` so it does not shadow the `flush()` function)
- Lines under 120 characters
- Warnings are errors in the library and the tests (`-Werror -Wall -Wextra -Wpedantic`, `/W4 /WX` on MSVC)

### Testing Guidelines
- Put functional tests in `logcoe_test.cpp` and concurrency tests in `logcoe_thread_test.cpp`
- Put tests for `std::expected` results in `logcoe_error_handling_test.cpp`
- A new public function also needs a case in `logcoe_strip_test.cpp`

### Pull Request Process

PRs target `main` and are squash-merged. The CI jobs above run on every PR.

### Commit Messages
- PR title: `[Category]: <title>`
- PR description: the major changes as bullet points
- The squashed commit uses the PR title as its title and the PR description as its message

Example:
```
[Time Format]: Add custom time format validation

- Validate strftime format strings before applying
- Return error for invalid formats
- Add tests for format validation
```

## Adding New Features

1. Update the public API in `include/logcoe.hpp` if needed.
2. Implement it in the anonymous namespace in `src/logcoe.cpp`, with the locking described below.
3. Add a no-op stub in the `NDEBUG` branch of the public wrappers in `src/logcoe.cpp`.
4. Add tests, covering both success and failure cases.
5. Update the README and the docs if behavior changed.

## Thread Safety Guidelines

1. Every function that accesses static state locks `g_mutex` for its full duration.
2. Do not nest locks. The single mutex is not recursive, so internal helpers must not take it again.
3. Add a thread safety test for new features that touch shared state.

## Questions?

Contact: nircoe@gmail.com
