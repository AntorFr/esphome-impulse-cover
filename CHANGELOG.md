# Changelog

All notable changes to the ESPHome Impulse Cover component will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added - CI/CD Infrastructure
- Comprehensive CI/CD pipeline similar to HeishaMon project
- Multi-platform automated testing (ESP8266/ESP32)
- Performance and compatibility testing workflows
- Automated maintenance and dependency management
- GitHub issue templates for better community support
- Dependabot configuration for automatic dependency updates
- Enhanced pull request template with detailed checklists
- Automated release workflow with asset generation
- Security scanning and vulnerability detection
- Code quality checks with multiple linters (Black, flake8, pylint, clang-tidy)
- Documentation validation and freshness checking
- Weekly maintenance automation
- Memory usage analysis and stress testing

### Enhanced - Test Infrastructure
- Quality check workflow with better error handling
- Test script integration in CI pipeline (`test-impulse-cover.sh`)
- Repository settings configuration
- Code formatting and style enforcement
- Comprehensive example validation

### Fixed - YAML Formatting and Configuration
- Removed trailing spaces from all example configuration files
- Fixed YAML syntax validation for all workflow files
- Added missing `.venv` pattern to .gitignore
- Corrected indentation issues in GitHub Actions workflows
- Ensured all YAML files pass yamllint validation

### Fixed - Binary Sensor and Partial Opening
- Restored missing binary sensor functionality with conditional compilation
- Added `#ifdef USE_BINARY_SENSOR` for backward compatibility
- Implemented configurable `open_duration` and `close_duration` parameters
- Enhanced position calculation with `start_position_` tracking
- Improved partial opening accuracy and control

## [1.0.0] - 2025-01-01

### Added
- Initial release of Impulse Cover component
- Single-pulse control logic for gates and garage doors
- Time-based position calculation with configurable durations
- Optional endstop sensor support for accurate positioning
- Safety timeout protection (configurable, default 60s)
- Cycling detection to prevent infinite open/close loops
- Configurable pulse delay for reverse operations (default 500ms)
- Home Assistant integration with position reporting and control
- Automation triggers for open, close, idle, and safety events
- Support for mid-movement direction reversal
- Automatic safety mode recovery after idle period
- Position state persistence across reboots
- ESP32 and ESP8266 platform support

### Features
- **Single Pulse Control**: One output controls all operations
  - Open when closed
  - Close when open  
  - Stop when moving
  - Reverse after stop in middle
  
- **Safety System**: 
  - Movement timeout protection
  - Rapid cycling detection
  - Automatic recovery mechanisms
  
- **Position Tracking**:
  - Time-based calculation
  - Endstop sensor correction
  - Percentage reporting to Home Assistant
  - Settable target positions
  
- **Hardware Support**:
  - Any GPIO output for pulses
  - Optional open/close sensors
  - Compatible with standard relay modules
  
- **Integration**:
  - Full Home Assistant cover entity
  - Standard cover controls (open/close/stop/toggle)
  - Position slider in UI
  - Device status reporting

### Configuration Options
- `output`: GPIO output for pulses (required)
- `open_duration`: Time to fully open (required) 
- `close_duration`: Time to fully close (required)
- `pulse_delay`: Delay between stop/reverse pulses (default: 500ms)
- `safety_timeout`: Maximum operation time (default: 60s)
- `safety_max_cycles`: Max direction changes before safety (default: 5)
- `open_sensor`: Optional fully-open sensor
- `close_sensor`: Optional fully-closed sensor
- Automation triggers: `on_open`, `on_close`, `on_idle`, `on_safety`

### Documentation
- Comprehensive README with examples
- Technical documentation for developers
- Example configuration file
- API reference documentation

### Tested Scenarios
- Gate controllers with single-button operation
- Garage door systems with simple control
- Sliding door mechanisms
- Motorized shutters
- Industrial door systems

## [0.9.0] - 2024-12-15

### Added
- Development version with basic functionality
- Core state machine implementation
- Basic pulse control logic
- Initial safety mechanisms

### Known Issues
- Position calculation needed refinement
- Safety system required additional testing
- Documentation incomplete

---

## Future Releases

### Planned for [1.1.0]
- Acceleration/deceleration curves for smoother movement
- Multiple position presets (25%, 50%, 75% positions)
- Enhanced obstruction detection
- Improved Home Assistant device information
- Additional automation triggers

### Planned for [1.2.0]  
- Advanced safety features (obstruction sensors)
- Wear monitoring and maintenance alerts
- Custom position algorithms
- Mobile app integration enhancements

### Planned for [2.0.0]
- Multi-motor support
- Advanced control algorithms
- Predictive maintenance
- Machine learning position optimization
