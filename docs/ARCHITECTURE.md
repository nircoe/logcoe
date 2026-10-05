# logcoe Architecture

## Overview

logcoe is a thread-safe logging library. The public API is a set of free functions. The implementation is free
functions and static state inside an anonymous namespace, so no implementation types are exposed.

## Component Architecture

```
                                        ┌─────────────────────────────────────┐
                                        │           User Application          │
                                        │            (Client Code)            │
                                        └──────────────────┬──────────────────┘
                                                           │
                                        ┌──────────────────▼──────────────────┐
                                        │              logcoe API             │
                                        │     (Public Interface Functions)    │
                                        └──────────────────┬──────────────────┘
                                                           │
                                        ┌──────────────────▼──────────────────┐
                                        │    Anonymous Namespace (Internal)   │
                                        │       (Free Functions & State)      │
                                        │                                     │
                                        │  ┌─────────────┬─────────────────┐  │
                                        │  │    Mutex    │  Output Streams │  │
                                        │  │ Protection  │   Management    │  │
                                        │  └─────────────┴─────────────────┘  │
                                        │  ┌─────────────┬─────────────────┐  │
                                        │  │  Log Level  │   Timestamp     │  │
                                        │  │  Filtering  │   Formatting    │  │
                                        │  └─────────────┴─────────────────┘  │
                                        └─────────────────────────────────────┘
```

## Core Components

### Public API Layer
- File: `include/logcoe.hpp`
- Purpose: the interface client code calls
- Function-based API, no implementation types exposed
- Single public header
- Every function forwards to the anonymous-namespace implementation

### Internal Implementation (Anonymous Namespace)
- File: `src/logcoe.cpp` (anonymous namespace)
- Purpose: all logging logic and state
- Free functions with internal linkage through namespace scope

#### Thread Safety Manager
```cpp
std::mutex g_mutex;
```
- Guards all state variables and operations

#### State Management
```cpp
log_level   g_log_level;
bool        g_use_file;
bool        g_use_console;
std::string g_time_format;
```
- Current logger configuration, can be changed at runtime
- The block above is partial. `g_init_counter` (`unsigned int`) counts `initialize` calls and
  `g_default_source` (`std::string`) holds the default source

#### Output Stream Management
```cpp
std::string     g_filename;
std::ofstream   g_file_stream;
std::ostream*   g_console_stream;
```
- File output: the `std::ofstream` is opened and closed by `initialize`, `set_file_output`, `disable_file_output` and
  `shutdown`
- Console output: configurable stream, `std::cout` by default

## Data Flow

### 1. Initialization Process

Each `initialize` call raises `g_init_counter`. Only the first call runs the steps below, later calls log
`[logcoe] Already initialized, ignoring new configurations`. `shutdown` lowers the counter and only the call that
brings it to 0 closes the streams and resets the state.

```
initialize() called
    ↓
Acquire mutex lock
    ↓
Set configuration parameters
    ↓
Open file stream (if enabled)
    ↓
Set console stream pointer
    ↓
Write initialization message
    ↓
Release mutex lock
```

### 2. Logging Process
```
debug/info/warning/error() called
    ↓
log() internal function
    ↓
Acquire mutex lock
    ↓
Generate timestamp
    ↓
Format message with metadata
    ↓
write_to_outputs()
    ↓
Check log level filtering
    ↓
Write to console (if enabled)
    ↓
Write to file (if enabled)
    ↓
Flush streams (if requested)
    ↓
Release mutex lock
```

### 3. Configuration Changes
```
set_log_level/set_file_output/etc() called
    ↓
Acquire mutex lock
    ↓
Flush existing streams (output changes only)
    ↓
Update configuration
    ↓
Reinitialize streams (if needed)
    ↓
Release mutex lock
```

## Thread Safety Implementation

- Single global mutex: `std::mutex g_mutex`
- Lock scope: every public API call holds the lock for its entire duration

### Thread Safety Guarantees
1. Configuration consistency: all threads see consistent logger state
2. Message integrity: log messages are not interleaved
3. Stream safety: no concurrent access to output streams
4. Atomic updates: configuration changes are atomic

## Cross-Platform Considerations

### Time Formatting
```cpp
std::time_t time_t_now = std::chrono::system_clock::to_time_t(now);
std::tm tm_now;
#ifdef _WIN32
    localtime_s(&tm_now, &time_t_now);
#else
    localtime_r(&time_t_now, &tm_now);
#endif
```
- Windows: `localtime_s`
- Linux and macOS: `localtime_r`
- Both are the thread-safe variants

### File System Operations
- Paths are handled with `std::filesystem`
- `initialize` creates missing parent directories and deletes an existing file at the path
- `set_file_output` opens the file for writing, which truncates an existing file
- With the default filename `logcoe.log`, `initialize` uses `logcoe_<timestamp>.log` instead
- File permissions are the OS defaults

### Build System Integration
- CMake: usable through FetchContent
- Language standard: C++23, for `std::expected`
- Library type: static
- Warnings are errors (`-Werror -Wall -Wextra -Wpedantic`, `/W4 /WX` on MSVC)

## Release Build Stripping

Under `NDEBUG`, the whole anonymous-namespace implementation is compiled out with `#ifndef NDEBUG`, so a Release
build carries none of its code or state. The public `logcoe::` functions in `src/logcoe.cpp` switch to a separate
`#ifdef NDEBUG` branch of no-op stubs, so call sites compile unchanged.

Four stubs return a fixed value because there is no state left to report:
- `is_initialized()` returns `false`
- `get_log_level()` returns `log_level::none`
- `set_file_output()` and `set_time_format()` return success, and no file is ever created

A stripped build never changes a caller's control flow. The guard is the bare `NDEBUG` macro, not the "Release"
build type, so `RelWithDebInfo` and `MinSizeRel` are stripped too, since CMake defines `NDEBUG` for them.

## Memory Management

### Static Storage
- Lifetime: all state is stored in static variables
- Activation: state becomes live on the first `initialize()`
- Cleanup: `shutdown()` closes the file and resets the state when the init counter reaches 0

### Resource Management
- File streams: RAII through `std::ofstream`

## Log Level Filtering

### Level Hierarchy
```
debug (0) < info (1) < warning (2) < error (3) < none (4)
```

```cpp
if (static_cast<int>(level) < static_cast<int>(g_log_level))
    return;
```

- Levels are compared as integers
- The check is in `write_to_outputs()`, after the message is formatted. Filtered messages are dropped there

## Message Formatting

### Format Structure
```
[timestamp] [LEVEL] [source]: <message>
```

- Timestamp: `strftime` format, configurable
- Level: string form of `log_level`
- Source: optional component identifier, falls back to the default source from `initialize`
- Message: the text passed by the caller

## Error Handling

### Stream Failures
- File open errors: logged to the outputs and file output is disabled.
  `set_file_output` returns `error_reason::file_open_failure`
- Write failures: ignored silently
- Invalid time format: the current format is kept and an error is logged.
  `set_time_format` returns `error_reason::invalid_time_format`. A format that formats to an empty string
  is invalid
- Not initialized: `set_file_output` and `set_time_format` do nothing and return
  `error_reason::not_initialized`

### Exception Safety
- The API reports failures through return values and does not throw by design
- File streams close through RAII
- The mutex keeps state consistent when an operation fails

## Performance Characteristics

- One global mutex serializes all calls, and formatting and I/O run under it
- Each message is written with `std::endl`, which flushes the stream, so the `flush` argument currently adds
  nothing
