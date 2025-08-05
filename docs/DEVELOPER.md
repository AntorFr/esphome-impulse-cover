# Developer Guide

This document provides detailed information for developers who want to contribute to the ESPHome Impulse Cover component.

## Development Environment Setup

### Prerequisites

- Python 3.8 or higher
- ESPHome 2025.7.4 or compatible version
- Git
- VS Code (recommended) with recommended extensions

### Quick Setup

1. **Clone the repository:**
   ```bash
   git clone https://github.com/berard/esphome-impulse-cover.git
   cd esphome-impulse-cover
   ```

2. **Set up Python environment:**
   ```bash
   python -m venv venv
   source venv/bin/activate  # On Windows: venv\Scripts\activate
   pip install esphome==2025.7.4 black flake8 pylint isort yamllint
   ```

3. **Open in VS Code:**
   ```bash
   code esphome-impulse-cover.code-workspace
   ```

### Development Container

For a consistent development environment, use the provided dev container:

1. Install Docker and VS Code with the Remote-Containers extension
2. Open the project in VS Code
3. When prompted, reopen in container or use Command Palette: "Remote-Containers: Reopen in Container"

## Project Structure

```
esphome-impulse-cover/
├── .devcontainer/          # Development container configuration
├── .github/                # GitHub workflows and templates
│   ├── workflows/          # CI/CD workflows
│   ├── ISSUE_TEMPLATE/     # Issue templates
│   └── PULL_REQUEST_TEMPLATE.md
├── components/             # Main component code
│   └── impulse_cover/      # Impulse cover component
│       ├── __init__.py     # Component registration
│       ├── cover.py        # Cover implementation
│       └── impulse_cover.h # C++ header
├── examples/               # Example configurations
│   ├── basic-configuration.yaml
│   ├── with-sensors.yaml
│   └── partial-test.yaml
├── docs/                   # Documentation
├── test-impulse-cover.sh   # Test script
├── pyproject.toml          # Python project configuration
└── README.md
```

## Development Workflow

### 1. Before Starting

- Create a new branch from `main`
- Ensure your development environment is set up
- Run the test suite to verify everything works

### 2. Making Changes

1. **Code Style:** Follow PEP 8 and use the configured formatters
   ```bash
   black components/ examples/
   isort components/ examples/
   ```

2. **Testing:** Always test your changes
   ```bash
   ./test-impulse-cover.sh
   ./test-impulse-cover.sh --compile
   ```

3. **Linting:** Ensure code quality
   ```bash
   flake8 components/ examples/
   pylint components/
   yamllint examples/
   ```

### 3. Testing

#### Unit Testing
- Run validation tests: `./test-impulse-cover.sh`
- Run compilation tests: `./test-impulse-cover.sh --compile`
- Test specific platforms: `./test-impulse-cover.sh --platform esp32`

#### Integration Testing
- Test with real hardware when possible
- Verify Home Assistant integration
- Test edge cases and error conditions

#### Example Configuration Testing
All example configurations should:
- Compile successfully for both ESP8266 and ESP32
- Follow best practices for ESPHome configuration
- Include comprehensive comments
- Demonstrate different use cases

### 4. Documentation

- Update README.md if adding new features
- Add or update code comments for complex logic
- Update CHANGELOG.md following semantic versioning
- Ensure all public APIs are documented

### 5. Submitting Changes

1. **Commit Guidelines:**
   - Use conventional commit format: `type(scope): description`
   - Types: `feat`, `fix`, `docs`, `style`, `refactor`, `test`, `chore`
   - Examples:
     - `feat(cover): add partial opening support`
     - `fix(sensor): resolve timing issue in position calculation`
     - `docs: update installation instructions`

2. **Pull Request:**
   - Fill out the PR template completely
   - Ensure all CI checks pass
   - Request review from maintainers

## Code Guidelines

### Python Code

- Follow PEP 8 style guide
- Use type hints where appropriate
- Maximum line length: 100 characters
- Use descriptive variable and function names
- Add docstrings for public methods and classes

### C++ Code

- Follow Google C++ Style Guide
- Use consistent indentation (2 spaces)
- Add header guards
- Document complex algorithms
- Prefer const correctness

### YAML Configuration

- Use 2-space indentation
- Follow ESPHome configuration patterns
- Add comments for complex configurations
- Validate syntax with yamllint

## Component Architecture

### Python Components

The Python component (`components/impulse_cover/__init__.py`) handles:
- Configuration validation using Voluptuous
- Code generation for C++ implementation
- Integration with ESPHome core systems

### C++ Implementation

The C++ component (`components/impulse_cover/impulse_cover.h`) provides:
- Real-time control logic
- Hardware abstraction
- State management
- Safety mechanisms

### Key Classes

1. **ImpulseCover:** Main cover component
2. **ImpulseCoverCall:** Cover operation implementation
3. **Position calculation logic:** State tracking and estimation

## Testing Strategy

### Automated Testing

1. **Syntax Validation:** YAML and Python syntax checking
2. **Compilation Tests:** Verify code compiles for target platforms
3. **Style Checks:** Code formatting and linting
4. **Configuration Validation:** Example configurations

### Manual Testing

1. **Hardware Testing:** Real device testing when possible
2. **Integration Testing:** Home Assistant integration
3. **Edge Case Testing:** Error conditions and recovery
4. **Performance Testing:** Response time and resource usage

## Release Process

### Version Management

- Follow semantic versioning (MAJOR.MINOR.PATCH)
- Update version in relevant files
- Tag releases in Git

### Release Checklist

1. [ ] All tests pass
2. [ ] Documentation updated
3. [ ] CHANGELOG.md updated
4. [ ] Version bumped
5. [ ] Create GitHub release
6. [ ] Verify release artifacts

### Hotfix Process

For critical bug fixes:
1. Create hotfix branch from latest release tag
2. Apply minimal fix
3. Test thoroughly
4. Create patch release
5. Merge back to main

## Troubleshooting

### Common Issues

1. **Compilation Errors:**
   - Check ESPHome version compatibility
   - Verify C++ syntax and includes
   - Check for missing dependencies

2. **Configuration Errors:**
   - Validate YAML syntax
   - Check parameter types and ranges
   - Verify platform compatibility

3. **Runtime Issues:**
   - Check logs for error messages
   - Verify hardware connections
   - Test timing parameters

### Debug Tools

- ESPHome logs: `esphome logs config.yaml`
- Serial monitor for direct debugging
- Home Assistant developer tools
- Logic analyzer for timing analysis

## Contributing Guidelines

### Code Review Process

1. All changes require review
2. Maintain backward compatibility
3. Follow established patterns
4. Document breaking changes

### Community Guidelines

- Be respectful and constructive
- Help other contributors
- Share knowledge and experience
- Follow the code of conduct

## Resources

### Documentation

- [ESPHome Documentation](https://esphome.io/)
- [Home Assistant Documentation](https://www.home-assistant.io/docs/)
- [Component Development Guide](https://esphome.io/guides/custom_device_class.html)

### Tools

- [ESPHome Dashboard](https://esphome.io/guides/getting_started_command_line.html)
- [PlatformIO](https://platformio.org/)
- [VS Code ESPHome Extension](https://marketplace.visualstudio.com/items?itemName=ESPHome.esphome-vscode)

### Community

- [ESPHome Discord](https://discord.gg/KhAMKrd)
- [Home Assistant Community Forum](https://community.home-assistant.io/)
- [GitHub Discussions](https://github.com/berard/esphome-impulse-cover/discussions)
