# logcoe

Thread-safe C++ logging library with real-time output and customizable formatting.

[![Windows](https://github.com/nircoe/logcoe/actions/workflows/ci-windows.yml/badge.svg)](https://github.com/nircoe/logcoe/actions/workflows/ci-windows.yml)
[![Linux](https://github.com/nircoe/logcoe/actions/workflows/ci-linux.yml/badge.svg)](https://github.com/nircoe/logcoe/actions/workflows/ci-linux.yml)
[![macOS](https://github.com/nircoe/logcoe/actions/workflows/ci-macos.yml/badge.svg)](https://github.com/nircoe/logcoe/actions/workflows/ci-macos.yml)

## What is logcoe?

logcoe is a lightweight, thread-safe C++ logging library designed for high-performance applications. It provides flexible output options and real-time logging with minimal overhead:

```
[2025-06-07_14:30:25] [INFO]: Application started successfully
[2025-06-07_14:30:25] [DEBUG] [NetworkManager]: Connecting to server 192.168.1.100
[2025-06-07_14:30:26] [WARNING] [Database]: Connection timeout, retrying...
[2025-06-07_14:30:27] [ERROR] [FileSystem]: Failed to open configuration file
```

Perfect for applications requiring reliable logging across multiple threads with customizable output destinations.

## Dependencies
- **C++17 or later** - Modern C++ standard support
- **CMake 3.14+** - Build system

## Quick Start

### 1. Add to your project (CMake)

```cmake
include(FetchContent)
FetchContent_Declare(
    logcoe
    GIT_REPOSITORY https://github.com/nircoe/logcoe.git
    GIT_TAG v0.1.0
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
    logcoe::initialize(logcoe::LogLevel::INFO, std::string{}, true, false);
    
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
    logcoe::initialize(logcoe::LogLevel::DEBUG, "logcoe", true, true, "app.log");
    
    // Customize time format
    logcoe::setTimeFormat("%H:%M:%S");
    
    // Log with source information
    logcoe::debug("Debugging network connection", "NetworkModule");
    logcoe::info("User logged in successfully", "AuthSystem");
    
    // Redirect console to custom stream
    std::ofstream customLog("custom.log");
    logcoe::setConsoleOutput(customLog);
    
    // Change log level at runtime
    logcoe::setLogLevel(logcoe::LogLevel::WARNING);
    
    logcoe::shutdown();
    return 0;
}
```

## Features

- 🔒 **Thread-Safe** - Concurrent logging from multiple threads
- 📊 **Multiple Log Levels** - DEBUG, INFO, WARNING, ERROR with runtime filtering
- 🖥️ **Dual Output** - Console and file output simultaneously
- ⚡ **High Performance** - Minimal overhead with optional flushing control
- 🎨 **Customizable** - Configurable time formats and output streams
- 🔄 **Dynamic Configuration** - Change settings during runtime
- 📦 **Zero Dependencies** - Header-only public API, pure C++17
- 🌐 **Cross-Platform** - Windows, Linux, macOS support

## API Reference

### Initialization
```cpp
// Basic initialization
logcoe::initialize();

// Full configuration
logcoe::initialize(
    logcoe::LogLevel::DEBUG,  // Log level
    "logcoe",                 //default source
    true,                     // Enable console
    true,                     // Enable file
    "application.log"         // Filename
);

// Clean shutdown
logcoe::shutdown();
```

### Configuration
```cpp
// Runtime log level changes
logcoe::setLogLevel(logcoe::LogLevel::WARNING);
LogLevel current = logcoe::getLogLevel();

// Output configuration
logcoe::setFileOutput("new_logfile.log");
logcoe::disableFileOutput();
logcoe::setConsoleOutput(std::cerr);
logcoe::disableConsoleOutput();

// Time formatting (strftime compatible)
logcoe::setTimeFormat("%Y-%m-%d %H:%M:%S");
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
| `DEBUG` | 0 | Detailed diagnostic information |
| `INFO` | 1 | General application information |
| `WARNING` | 2 | Warning conditions that should be noted |
| `ERROR` | 3 | Error conditions that affect functionality |
| `NONE` | 4 | Disable all logging |

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
    logcoe::initialize(logcoe::LogLevel::INFO,  std::string{}, false, true, "concurrent.log");
    
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

- **Compiler**: C++17 compatible (GCC 7+, Clang 5+, MSVC 2017+)
- **Build System**: CMake 3.14+
- **Platforms**: Windows, Linux, macOS

## Performance Considerations

- **Flushing**: Set `flush=false` for high-frequency logging to improve performance
- **Log Levels**: Higher log levels filter out lower-priority messages at minimal cost
- **Thread Contention**: Minimal mutex contention with efficient lock granularity

## Documentation

- [Architecture](docs/ARCHITECTURE.md) - Internal design and implementation details
- [Contributing](docs/CONTRIBUTING.md) - Development setup and contribution guidelines
- [Roadmap](docs/ROADMAP.md) - Version history and planned features

## License

MIT License - see [LICENSE](LICENSE) file for details.
