# logcoe Roadmap

## Version History

### v0.1.0 - Initial Release
- Thread-safe logging with mutex protection
- Multiple log levels (DEBUG, INFO, WARNING, ERROR, NONE)
- Dual output support (console and file simultaneously)
- Dynamic log level changes at runtime
- Customizable time formatting with strftime compatibility
- Output stream redirection
- Cross-platform support (Windows, Linux, macOS)
- Optional flushing control
- Source field support for component identification
- Test suite including thread safety tests
- CMake integration with FetchContent support
- Support for MSVC, GCC, Clang, and MinGW compilers

### v0.1.1 - Fixes and Polish
- Add `[logcoe]` prefix to internal messages
- Change default time format
- Change default log level to DEBUG
- Correct `initialize` arguments in the README
- Promote testcoe to v0.1.1

## Future Plans

- Asynchronous logging
- Custom log formatters and templates
- ANSI color support for console output
- Log filtering by source or pattern
- Multiple simultaneous log files
- Log compression and archival

## Feature Requests

Open an issue on GitHub with the "enhancement" label, or write to nircoe@gmail.com.

## Versioning

logcoe follows [Semantic Versioning](https://semver.org/):
- MAJOR version for incompatible API changes
- MINOR version for backwards-compatible functionality additions
- PATCH version for backwards-compatible bug fixes