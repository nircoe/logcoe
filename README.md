# logcoe

Thread-safe C++ logging library with real-time output and customizable formatting.

[![Windows](https://github.com/nircoe/logcoe/actions/workflows/ci-windows.yml/badge.svg)](https://github.com/nircoe/logcoe/actions/workflows/ci-windows.yml)
[![Linux](https://github.com/nircoe/logcoe/actions/workflows/ci-linux.yml/badge.svg)](https://github.com/nircoe/logcoe/actions/workflows/ci-linux.yml)
[![macOS](https://github.com/nircoe/logcoe/actions/workflows/ci-macos.yml/badge.svg)](https://github.com/nircoe/logcoe/actions/workflows/ci-macos.yml)

## What is logcoe?

logcoe is a lightweight, thread-safe C++ logging library designed for high-performance applications. It provides flexible output options and real-time logging with minimal overhead:

```
[06/07/2025__14:30:25] [INFO]: Application started successfully
[06/07/2025__14:30:25] [DEBUG] [NetworkManager]: Connecting to server 192.168.1.100
[06/07/2025__14:30:26] [WARNING] [Database]: Connection timeout, retrying...
[06/07/2025__14:30:27] [ERROR] [FileSystem]: Failed to open configuration file
```

Perfect for applications requiring reliable logging across multiple threads with customizable output destinations.

## Dependencies
- **C++23 or later** - Modern C++ standard support
- **CMake 3.22+** - Build system

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

- **Thread-Safe** - Concurrent logging from multiple threads
- **Multiple Log Levels** - DEBUG, INFO, WARNING, ERROR with runtime filtering
- **Dual Output** - Console and file output simultaneously
- **High Performance** - Minimal overhead with optional flushing control
- **Customizable** - Configurable time formats and output streams
- **Dynamic Configuration** - Change settings during runtime
- **Zero Dependencies** - Header-only public API, pure C++23
- **Cross-Platform** - Windows, Linux, macOS support

## API Reference

### Initialization
```cpp
// Basic initialization
logcoe::initialize();

// Full configuration
logcoe::initialize(
    logcoe::log_level::debug,  // Log level
    "logcoe",                  // Default source
    true,                      // Enable console
    true,                      // Enable file
    "application.log"          // Filename
);

// shutdown
logcoe::shutdown();
```

### Configuration
```cpp
// Runtime log level changes
logcoe::set_log_level(logcoe::log_level::warning);
log_level current = logcoe::get_log_level();

// Output configuration
// set_file_output and set_time_format return std::expected<void, logcoe::error_reason>
if (auto result = logcoe::set_file_output("new_logfile.log"); !result)
{
    // result.error() is logcoe::error_reason::file_open_failure
}
logcoe::disable_file_output();
logcoe::set_console_output(std::cerr);
logcoe::disable_console_output();

// Time formatting (strftime compatible)
if (auto result = logcoe::set_time_format("%Y-%m-%d %H:%M:%S"); !result)
{
    // result.error() is logcoe::error_reason::invalid_time_format
}
```

### Logging
```cpp
// Basic logging
logcoe::debug("Debug message");
logcoe::info("Information message");
logcoe::warning("Warning message");
logcoe::error("Error message");

// With source identification
logcoe::info("User action completed", "UserController");

// Control flushing for performance
logcoe::info("High frequency message", "", false);  // No immediate flush
logcoe::flush();  // Flush all pending messages
```

## Log Levels

| Level | Value | Description |
|-------|-------|-------------|
| `debug` | 0 | Detailed diagnostic information |
| `info` | 1 | General application information |
| `warning` | 2 | Warning conditions that should be noted |
| `error` | 3 | Error conditions that affect functionality |
| `none` | 4 | Disable all logging |

## Thread Safety

logcoe is fully thread-safe and designed for high-concurrency environments:

```cpp
#include <thread>
#include <vector>

void worker_thread(int id) {
    for (int i = 0; i < 1000; ++i) {
        logcoe::info("Worker " + std::to_string(id) + " processing item " + std::to_string(i));
    }
}

int main() {
    logcoe::initialize(logcoe::log_level::info, std::string{}, false, true, "concurrent.log");
    
    std::vector<std::thread> workers;
    for (int i = 0; i < 10; ++i) {
        workers.emplace_back(worker_thread, i);
    }
    
    for (auto& t : workers) {
        t.join();
    }
    
    logcoe::shutdown();
    return 0;
}
```

## Requirements

- **Compiler**: C++23 compatible
- **Build System**: CMake 3.22+
- **Platforms**: Windows, Linux, macOS

## Performance Considerations

- **Flushing**: Set `flush=false` for high-frequency logging to improve performance
- **Log Levels**: Higher log levels filter out lower-priority messages at minimal cost
- **Thread Contention**: Minimal mutex contention with efficient lock granularity

## Release Builds

Defining `NDEBUG` (e.g. `-DCMAKE_BUILD_TYPE=Release`) strips logcoe down to no-op stubs at compile
time. Every call site keeps working with zero code changes, but none of the logging, formatting,
or file I/O gets compiled in:

- `initialize`, `debug`/`info`/`warning`/`error`, `flush`, `shutdown`, and the setters do nothing.
- `is_initialized()` returns `false`.
- `get_log_level()` returns `log_level::none`.
- `set_file_output(...)` returns `error_reason::file_open_failure` and never creates a file.
- `set_time_format(...)` returns success.

No opt-in macro or CMake option needed, it's automatic whenever `NDEBUG` is defined. That includes
`RelWithDebInfo` and `MinSizeRel`, since CMake defines `NDEBUG` for those build types too, not just
`Release`.

## Documentation

- [Architecture](docs/ARCHITECTURE.md) - Internal design and implementation details
- [Contributing](docs/CONTRIBUTING.md) - Development setup and contribution guidelines
- [Roadmap](docs/ROADMAP.md) - Version history and planned features

## License

MIT License - see [LICENSE](LICENSE) file for details.
