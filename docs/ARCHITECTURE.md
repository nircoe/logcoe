# logcoe Architecture

## Overview

logcoe is designed as a lightweight, thread-safe logging library that provides flexible output management with minimal performance overhead. The architecture follows a singleton pattern with internal implementation hiding for API stability.

## Component Architecture

```
┌─────────────────────────────────────┐
│         User Application            │
│      (Client Code)                  │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│            logcoe API               │
│    (Public Interface Functions)     │
└───────────────┬─────────────────────┘
                │
┌───────────────▼─────────────────────┐
│           LoggerImpl                │
│    (Internal Implementation)        │
│                                     │
│  ┌─────────────┬─────────────────┐  │
│  │   Mutex     │  Output Streams │  │
│  │ Protection  │   Management    │  │
│  └─────────────┴─────────────────┘  │
│                                     │
│  ┌─────────────┬─────────────────┐  │
│  │ Log Level   │   Timestamp     │  │
│  │ Filtering   │   Formatting    │  │
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
  - All functions forward to LoggerImpl

### LoggerImpl (Internal Implementation)
- **File**: `src/logcoe.cpp` (anonymous namespace)
- **Purpose**: Contains all logging logic and state management
- **Design Pattern**: Singleton with static members

#### Thread Safety Manager
```cpp
static std::mutex s_mutex;
```
- **Purpose**: Ensures thread-safe access to all shared state
- **Scope**: Protects all static members and operations
- **Granularity**: Single global mutex for simplicity and correctness

#### State Management
```cpp
static LogLevel s_logLevel;
static bool s_useFile;
static bool s_useConsole;
static std::string s_timeFormat;
```
- **Purpose**: Maintains current logger configuration
- **Thread Safety**: All access protected by mutex
- **Dynamic Updates**: Can be changed at runtime

#### Output Stream Management
```cpp
static std::string s_filename;
static std::ofstream s_fileStream;
static std::ostream* s_consoleStream;
```
- **File Output**: Direct file stream management with automatic opening/closing
- **Console Output**: Configurable output stream (default: std::cout)
- **Stream Safety**: All stream operations are mutex-protected

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
writeToOutputs()
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
setLogLevel/setFileOutput/etc() called
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

### Mutex Strategy
- **Single Global Mutex**: `std::mutex s_mutex`
- **Lock Scope**: Every public API call acquires lock for entire duration
- **Benefits**: 
  - Simple design, no deadlock risk
  - Guaranteed consistency across all operations
  - Atomic configuration changes

### Performance Considerations
- **Lock Granularity**: Coarse-grained locking for simplicity
- **Lock Duration**: Minimal time holding locks
- **Flush Control**: Optional flushing to reduce I/O under lock

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
- **Format**: Standard `strftime` formatting across platforms

### File System Operations
- **Path Handling**: Uses standard C++ filesystem operations
- **File Permissions**: Relies on OS default permissions
- **Directory Creation**: Not automatic, requires existing directories

### Build System Integration
- **CMake**: FetchContent compatible
- **Compiler Support**: C++17 standard requirements
- **Library Type**: Static library

## Memory Management

### Static Storage
- **Lifetime**: All state stored in static variables
- **Initialization**: Lazy initialization through first API call
- **Cleanup**: Explicit cleanup through `shutdown()`

### Resource Management
- **File Streams**: RAII through std::ofstream
- **Memory Allocation**: No dynamic allocation for core operations
- **Exception Safety**: Basic exception safety guarantees

## Log Level Filtering

### Filtering Logic
```cpp
if (static_cast<int>(level) < static_cast<int>(s_logLevel))
    return;
```
- **Numeric Comparison**: Log levels assigned integer values
- **Early Return**: Filtered messages exit immediately
- **Performance**: O(1) filtering with minimal overhead

### Level Hierarchy
```
DEBUG (0) < INFO (1) < WARNING (2) < ERROR (3) < NONE (4)
```

## Message Formatting

### Format Structure
```
[timestamp] [LEVEL] [source]: message
```

### Components
- **Timestamp**: Configurable format using strftime
- **Level**: String representation of LogLevel enum
- **Source**: Optional component identifier
- **Message**: User-provided content

## Error Handling

### Stream Failures
- **File Open Errors**: Logged to console, file output disabled
- **Write Failures**: Silent failure, no cascading errors
- **Configuration Errors**: Invalid settings ignored with warnings

### Exception Safety
- **No Throws**: Public API designed to never throw
- **Resource Safety**: RAII ensures proper resource cleanup
- **State Consistency**: Mutex ensures consistent state even with errors

## Performance Characteristics

### Time Complexity
- **Logging**: O(1) for level filtering, O(log_message_length) for formatting
- **Configuration**: O(1) for most operations
- **Thread Contention**: Minimal with short lock durations

### Space Complexity
- **Memory Usage**: Fixed overhead independent of message count
- **File Growth**: Linear with number of logged messages
- **Buffer Management**: No internal buffering beyond stream buffers