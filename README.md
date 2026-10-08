# logcoe

Thread-safe C++ logging library with console and file output and a configurable time format.

[![Windows](https://github.com/nircoe/logcoe/actions/workflows/ci-windows.yml/badge.svg)](https://github.com/nircoe/logcoe/actions/workflows/ci-windows.yml)
[![Linux](https://github.com/nircoe/logcoe/actions/workflows/ci-linux.yml/badge.svg)](https://github.com/nircoe/logcoe/actions/workflows/ci-linux.yml)
[![macOS](https://github.com/nircoe/logcoe/actions/workflows/ci-macos.yml/badge.svg)](https://github.com/nircoe/logcoe/actions/workflows/ci-macos.yml)

## What is logcoe?

logcoe is a small thread-safe C++ logging library. It writes to the console, to a file or to both:

```
[06/07/2025__14:30:25] [INFO]: Application started successfully
[06/07/2025__14:30:25] [DEBUG] [NetworkManager]: Connecting to server 192.168.1.100
[06/07/2025__14:30:26] [WARNING] [Database]: Connection timeout, retrying...
[06/07/2025__14:30:27] [ERROR] [FileSystem]: Failed to open configuration file
```

## Dependencies

No third-party libraries. Building the tests fetches [testcoe](https://github.com/nircoe/testcoe) from source.
Toolchain versions are under [Requirements](#requirements).

## Quick Start

### 1. Add to your project (CMake)

```cmake
include(FetchContent)
FetchContent_Declare(
    logcoe
    GIT_REPOSITORY https://github.com/nircoe/logcoe.git
    GIT_TAG v0.1.1
)
FetchContent_MakeAvailable(logcoe)

target_link_libraries(your_target PRIVATE logcoe)
```

### 2. Use in your application

```cpp
#include <logcoe.hpp>

int main() {
    // Initialize with INFO level, no default source (empty string), console enabled, file disabled
    // logs will be printed as [timestamp] [log_level]: <log_message>
    logcoe::initialize(logcoe::log_level::info, std::string{}, true, false);
    
    logcoe::info("Application started");
    logcoe::warning("This is a warning message");
    logcoe::error("Critical error occurred", "ErrorHandler");
    
    logcoe::shutdown();
    return 0;
}
```

### 3. Advanced configuration

```cpp
#include <logcoe.hpp>
#include <fstream>

int main() {
    // Enable both console and file output with DEBUG level and default source as logcoe
    // logs will be printed as [timestamp] [log_level] [logcoe]: <log_message>
    logcoe::initialize(logcoe::log_level::debug, "logcoe", true, true, "app.log");
    
    // Customize time format
    if (!logcoe::set_time_format("%H:%M:%S"))
        return 1;
    
    // Log with source information
    logcoe::debug("Debugging network connection", "NetworkModule");
    logcoe::info("User logged in successfully", "AuthSystem");
    
    // Redirect console to custom stream
    std::ofstream customLog("custom.log");
    logcoe::set_console_output(customLog);
    
    // Change log level at runtime
    logcoe::set_log_level(logcoe::log_level::warning);
    
    logcoe::shutdown();
    return 0;
}
```

## Features

- Thread-safe, one mutex guards every call
- Log levels debug, info, warning and error, filtered at runtime
- Console and file output at the same time
- Log level, console stream, log file and time format can change at runtime
- One public header, built as a static library
- Windows, Linux and macOS
- Compiles to no-op stubs in Release builds, see [Release Builds](#release-builds)

## API Reference

All functions are declared in `include/logcoe.hpp`, with their default arguments.

- `initialize` can be called more than once. Later calls ignore their arguments and only raise a counter.
  Logging stops after the matching number of `shutdown` calls.
- The logging functions and the setters do nothing before `initialize`.
- The logging functions take an optional `source` and a `flush` flag (default `true`).
  `logcoe::flush()` flushes the console and file outputs.
- `set_file_output` and `set_time_format` return `std::expected<void, logcoe::error_reason>` and are `[[nodiscard]]`.
- `error_reason` is `file_open_failure`, `invalid_time_format` or `not_initialized`.
  Both functions return `not_initialized` when called before `initialize`.
- Time formats use `strftime` syntax.

## Log Levels

| Level | Value | Description |
|-------|-------|-------------|
| `debug` | 0 | Detailed diagnostic information |
| `info` | 1 | General application information |
| `warning` | 2 | Warning conditions that should be noted |
| `error` | 3 | Error conditions that affect functionality |
| `none` | 4 | Disable all logging |

## Thread Safety

All functions can be called from any thread. A single mutex is held for the whole of each call,
see [Architecture](docs/ARCHITECTURE.md).

## Requirements

- Compiler: C++23 with `<expected>` (GCC 12+, Clang 16+ with libc++, Apple Clang from Xcode 15+, MSVC 2022 17.3+)
- Build system: CMake 3.22+
- Platforms: Windows, Linux, macOS

## Release Builds

Defining `NDEBUG` (for example `-DCMAKE_BUILD_TYPE=Release`) replaces every function with a no-op stub at
compile time. Call sites compile unchanged, but nothing is logged, formatted or written to a file:

- `initialize`, `debug`/`info`/`warning`/`error`, `flush`, `shutdown` and the other setters do nothing.
- `is_initialized()` returns `false`.
- `get_log_level()` returns `log_level::none`.
- `set_file_output(...)` returns success and never creates a file.
- `set_time_format(...)` returns success.

This is automatic whenever `NDEBUG` is defined, no option is needed. It includes `RelWithDebInfo` and
`MinSizeRel`, because CMake defines `NDEBUG` for those build types too.

## Documentation

- [Architecture](docs/ARCHITECTURE.md): how it works
- [Contributing](docs/CONTRIBUTING.md): build, test and PR rules
- [Roadmap](docs/ROADMAP.md): version history and planned features

## License

MIT License, see [LICENSE](LICENSE).
