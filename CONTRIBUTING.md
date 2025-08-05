# Contributing to ESPHome Impulse Cover

Thank you for your interest in contributing to the ESPHome Impulse Cover component! This project welcomes contributions from the community.

## Code of Conduct

This project adheres to a code of conduct that we expect all contributors to follow. Please read and follow these guidelines to ensure a welcoming environment for everyone.

### Our Standards

- Be respectful and inclusive
- Welcome newcomers and help them learn
- Focus on constructive feedback
- Show empathy towards other community members
- Respect differing viewpoints and experiences

## How to Contribute

### Reporting Issues

Before creating an issue, please:

1. **Search existing issues** to avoid duplicates
2. **Use the issue templates** provided
3. **Provide detailed information** including:
   - ESPHome version
   - Hardware platform (ESP8266/ESP32)
   - Configuration that causes the issue
   - Expected vs actual behavior
   - Log output if relevant

### Suggesting Features

We welcome feature suggestions! Please:

1. **Check existing feature requests** first
2. **Use the feature request template**
3. **Describe the use case** clearly
4. **Explain why this would benefit users**
5. **Consider backward compatibility**

### Contributing Code

#### Getting Started

1. **Fork the repository** on GitHub
2. **Clone your fork** locally
3. **Create a feature branch** from `main`
4. **Set up the development environment** (see [DEVELOPER.md](docs/DEVELOPER.md))

#### Development Process

1. **Make your changes**
   - Follow the coding standards
   - Add tests for new functionality
   - Update documentation as needed

2. **Test thoroughly**
   ```bash
   # Run validation tests
   ./test-impulse-cover.sh
   
   # Run compilation tests
   ./test-impulse-cover.sh --compile
   
   # Test specific platforms
   ./test-impulse-cover.sh --platform esp32
   ```

3. **Ensure code quality**
   ```bash
   # Format code
   black components/ examples/
   isort components/ examples/
   
   # Lint code
   flake8 components/ examples/
   pylint components/
   yamllint examples/
   ```

4. **Commit your changes**
   - Use [conventional commit](https://www.conventionalcommits.org/) format
   - Include detailed description
   - Reference issues if applicable

5. **Push to your fork** and **create a pull request**

#### Pull Request Guidelines

- **Fill out the PR template** completely
- **Link related issues** using keywords (fixes #123)
- **Provide clear description** of changes
- **Include testing instructions** if needed
- **Ensure CI checks pass**
- **Keep PRs focused** - one feature/fix per PR
- **Update documentation** if needed

### Code Style Guidelines

#### Python Code

- Follow [PEP 8](https://pep8.org/) style guide
- Use [Black](https://black.readthedocs.io/) for formatting
- Maximum line length: 100 characters
- Use meaningful variable names
- Add type hints where appropriate
- Include docstrings for public APIs

Example:
```python
def calculate_position(
    duration: float, 
    total_time: float, 
    direction: str
) -> float:
    """Calculate cover position based on movement duration.
    
    Args:
        duration: Time the cover has been moving (seconds)
        total_time: Total time for full open/close (seconds)
        direction: Movement direction ('open' or 'close')
        
    Returns:
        Position as float between 0.0 (closed) and 1.0 (open)
    """
    # Implementation here
    pass
```

#### C++ Code

- Follow [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)
- Use 2-space indentation
- Use header guards
- Prefer `const` correctness
- Document complex algorithms

Example:
```cpp
class ImpulseCover : public cover::Cover, public Component {
 public:
  void setup() override;
  void loop() override;
  void control(const cover::CoverCall &call) override;
  
  // Setters for configuration
  void set_open_duration(uint32_t duration) { this->open_duration_ = duration; }
  void set_close_duration(uint32_t duration) { this->close_duration_ = duration; }
  
 protected:
  uint32_t open_duration_{10000};   // Default 10 seconds
  uint32_t close_duration_{10000};  // Default 10 seconds
  // ... other members
};
```

#### YAML Configuration

- Use 2-space indentation
- Follow ESPHome patterns
- Add helpful comments
- Validate with yamllint

Example:
```yaml
# Example configuration for impulse cover
cover:
  - platform: impulse_cover
    name: "Garage Door"
    open_duration: 12s      # Time to fully open
    close_duration: 10s     # Time to fully close
    open_action:
      - switch.turn_on: relay_open
      - delay: 500ms
      - switch.turn_off: relay_open
    close_action:
      - switch.turn_on: relay_close
      - delay: 500ms
      - switch.turn_off: relay_close
```

### Testing Guidelines

#### Required Tests

1. **Configuration validation** - All example configs must compile
2. **Platform compatibility** - Test on ESP8266 and ESP32
3. **Functionality tests** - Verify core features work
4. **Edge cases** - Test error conditions and recovery

#### Testing Checklist

- [ ] All example configurations compile successfully
- [ ] Code passes linting (flake8, pylint, yamllint)
- [ ] Code is properly formatted (black, isort)
- [ ] Component works with latest ESPHome version
- [ ] No breaking changes to existing configurations
- [ ] Documentation updated if needed

### Documentation Guidelines

#### Code Documentation

- Document all public APIs
- Explain complex algorithms
- Include usage examples
- Comment non-obvious code

#### User Documentation

- Keep README.md up to date
- Update configuration examples
- Document breaking changes
- Explain troubleshooting steps

### Commit Guidelines

Use [conventional commits](https://www.conventionalcommits.org/) format:

```
<type>(<scope>): <description>

[optional body]

[optional footer]
```

#### Types

- `feat`: New feature
- `fix`: Bug fix
- `docs`: Documentation only changes
- `style`: Code style changes (formatting, missing semicolons, etc)
- `refactor`: Code change that neither fixes a bug nor adds a feature
- `test`: Adding missing tests or correcting existing tests
- `chore`: Changes to build process or auxiliary tools

#### Examples

```
feat(cover): add partial opening support

Allow covers to open to specific positions using the position parameter.
This enables more precise control for use cases like ventilation.

Closes #123
```

```
fix(sensor): resolve timing issue in position calculation

The position calculation was using incorrect timing when the cover
direction changed rapidly. This fix adds proper state tracking.

Fixes #456
```

### Release Process

Releases follow [semantic versioning](https://semver.org/):

- **MAJOR**: Breaking changes
- **MINOR**: New features (backward compatible)
- **PATCH**: Bug fixes (backward compatible)

#### Release Checklist

1. Update CHANGELOG.md
2. Update version numbers
3. Create GitHub release
4. Update documentation
5. Announce in community channels

### Community

#### Getting Help

- **GitHub Discussions**: For questions and general discussion
- **Issues**: For bug reports and feature requests
- **ESPHome Discord**: For real-time community support
- **Home Assistant Forum**: For integration questions

#### Helpful Resources

- [ESPHome Documentation](https://esphome.io/)
- [ESPHome Custom Component Guide](https://esphome.io/guides/custom_device_class.html)
- [Home Assistant Cover Integration](https://www.home-assistant.io/integrations/cover/)
- [Developer Guide](docs/DEVELOPER.md)

### Recognition

Contributors are recognized in:
- GitHub contributors list
- CHANGELOG.md for significant contributions
- Special thanks in release notes

### Questions?

If you have questions about contributing, please:

1. Check the [Developer Guide](docs/DEVELOPER.md)
2. Search existing discussions and issues
3. Create a new discussion for general questions
4. Create an issue for specific problems

Thank you for contributing to ESPHome Impulse Cover! 🎉
