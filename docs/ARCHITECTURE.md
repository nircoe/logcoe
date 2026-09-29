# logcoe Architecture

## Overview

logcoe is designed as a lightweight, thread-safe logging library that provides flexible output management with minimal performance overhead. The public API hides all implementation details through an anonymous namespace containing free functions and static state, maintaining API stability across changes.

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
- **File**: `include/logcoe.hpp`
- **Purpose**: Provides clean, stable interface for client applications
- **Key Features**:
  - Simple function-based API
  - No exposed implementation details
  - Header-only public interface
  - All functions forward to anonymous-namespace implementations

### Internal Implementation (Anonymous Namespace)
- **File**: `src/logcoe.cpp` (anonymous namespace)
- **Purpose**: Contains all logging logic and state management
- **Design**: Free functions with implicit internal linkage through namespace scope

#### Thread Safety Manager
```cpp
std::mutex g_mutex;
```
- Ensures thread-safe access to all state variables and operations

#### State Management
```cpp
log_level   g_log_level;
bool        g_use_file;
bool        g_use_console;
std::string g_time_format;
```
- Maintains current logger configuration, can be changed at runtime

#### Output Stream Management
```cpp
std::string     g_filename;
std::ofstream   g_file_stream;
std::ostream*   g_console_stream;
```
- **File Output**: Direct file stream management with automatic opening/closing
- **Console Output**: Configurable output stream (default: std::cout)

## Data Flow

### 1. Initialization Process
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
Check log level filtering
    ↓
Generate timestamp
    ↓
Format message with metadata
    ↓
write_to_outputs()
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
Flush existing streams
    ↓
Update configuration
    ↓
Reinitialize streams (if needed)
    ↓
Release mutex lock
```

## Thread Safety Implementation

- **Single Global Mutex**: `std::mutex g_mutex`
- **Lock Scope**: Every public API call acquires lock for entire duration

### Thread Safety Guarantees
1. **Configuration Consistency**: All threads see consistent logger state
2. **Message Integrity**: No interleaved log messages
3. **Stream Safety**: No concurrent access to output streams
4. **Atomic Updates**: Configuration changes are atomic

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
- **Windows**: Uses `localtime_s` for thread safety
- **Unix/Linux/macOS**: Uses `localtime_r` for thread safety

### File System Operations
- **Path Handling**: Uses standard C++ filesystem operations
- **File Permissions**: Relies on OS default permissions

### Build System Integration
- **CMake**: FetchContent compatible
- **Compiler Support**: C++23 standard requirements
- **Library Type**: Static library

## Release Build Stripping

Under `NDEBUG`, the entire anonymous-namespace implementation is compiled out via
`#ifndef NDEBUG`, so a Release build carries none of its code or state variables. The public
`logcoe::` wrapper functions in `src/logcoe.cpp` switch to a separate `#ifdef NDEBUG` branch of
no-op stubs, so every call site keeps compiling unchanged. Two stubs return a fixed value instead
of an empty body, since there's no real state left to report: `is_initialized()` always returns
`false`, and `get_log_level()` always returns `log_level::none`. The guard is on the bare `NDEBUG`
macro, not a check for a "Release" build type specifically, so `RelWithDebInfo` and `MinSizeRel`
trigger the same stripping since CMake defines `NDEBUG` for them too.

## Memory Management

### Static Storage
- **Lifetime**: All state stored in static variables
- **Initialization**: Lazy initialization through `initialize()`
- **Cleanup**: Explicit cleanup through `shutdown()`

### Resource Management
- **File Streams**: RAII through std::ofstream
- **Memory Allocation**: No dynamic allocation for core operations
- **Exception Safety**: Basic exception safety guarantees

## Log Level Filtering

### Level Hierarchy
```
debug (0) < info (1) < warning (2) < error (3) < none (4)
```

```cpp
if (static_cast<int>(level) < static_cast<int>(g_log_level))
    return;
```

- **Numeric Comparison**: Log levels assigned integer values
- **Early Return**: Filtered messages exit immediately

## Message Formatting

### Format Structure
```
[timestamp] [LEVEL] [source]: <message>
```

- **Timestamp**: Configurable format using strftime
- **Level**: String representation of log_level enum
- **Source**: Optional component identifier
- **Message**: User-provided content

## Error Handling

### Stream Failures
- **File Open Errors**: Logged to console, file output disabled
- **Write Failures**: Silent failure, no exceptions
- **Configuration Errors**: Invalid settings ignored with warnings

### Exception Safety
- **No Exceptions**: Public API designed to never throw exceptions
- **Resource Safety**: RAII ensures proper resource cleanup
- **State Consistency**: Mutex ensures consistent state even with errors

## Performance Characteristics

### Time Complexity
- **Logging**: O(1) for level filtering, O(log_message_length) for formatting
- **Configuration**: O(1) for most operations
- **Thread Contention**: Minimal with short lock durations
