# logcoe Roadmap

## Version History

### v0.1.0 - Initial Release
- ✅ Thread-safe logging with mutex protection
- ✅ Multiple log levels (DEBUG, INFO, WARNING, ERROR, NONE)
- ✅ Dual output support (console and file simultaneously)
- ✅ Dynamic log level changes at runtime
- ✅ Customizable time formatting with strftime compatibility
- ✅ Output stream redirection capabilities
- ✅ Cross-platform support (Windows, Linux, macOS)
- ✅ Optional flushing control for performance optimization
- ✅ Source field support for component identification
- ✅ Comprehensive test suite with thread safety validation
- ✅ CMake integration with FetchContent support
- ✅ Support for MSVC, GCC, Clang, and MinGW compilers

## Future Plans

- ⏳ Log file rotation based on size or time
- ⏳ Asynchronous logging for high-performance scenarios
- ⏳ Structured logging (JSON/XML output formats)
- ⏳ Custom log formatters and templates
- ⏳ ANSI color support for console output
- ⏳ Network logging (UDP/TCP remote logging)
- ⏳ Syslog integration for Unix systems
- ⏳ Windows Event Log integration
- ⏳ Configuration file support
- ⏳ Log filtering by source or pattern
- ⏳ Multiple simultaneous log files
- ⏳ Lock-free logging implementation
- ⏳ Performance monitoring and statistics
- ⏳ Log compression and archival
- ⏳ Plugin architecture for custom outputs

## Feature Requests

Have an idea for logcoe? Please open an issue on GitHub with the "enhancement" label.

## Versioning

logcoe follows [Semantic Versioning](https://semver.org/):
- MAJOR version for incompatible API changes
- MINOR version for backwards-compatible functionality additions
- PATCH version for backwards-compatible bug fixes