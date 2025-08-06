# Pull Request

## Description
Brief description of the changes made.

## Type of Change
Please check the type of change your PR introduces:
- [ ] Bug fix (non-breaking change which fixes an issue)
- [ ] New feature (non-breaking change which adds functionality)
- [ ] Breaking change (fix or feature that would cause existing functionality to not work as expected)
- [ ] Documentation update
- [ ] Code improvement/refactoring
- [ ] Test improvement

## Testing
Please describe the tests that you ran to verify your changes:
- [ ] Ran `./create-validated-pr.sh --preview` to validate all configurations
- [ ] Tested all example configurations with ESPHome
- [ ] Tested with basic configuration
- [ ] Tested with sensor configuration  
- [ ] Tested with partial opening configuration
- [ ] Tested on ESP8266
- [ ] Tested on ESP32

## Configuration
If this change affects configuration, please provide example:

```yaml
# Example configuration showing the change
impulse_cover:
  - platform: impulse_cover
    name: "My Cover"
    # new/changed configuration here
```

## Breaking Changes
If this is a breaking change, please describe the impact and migration path:

## Checklist
- [ ] My code follows the project's style guidelines
- [ ] I have performed a self-review of my own code
- [ ] I have commented my code, particularly in hard-to-understand areas
- [ ] I have updated documentation if needed
- [ ] My changes generate no new warnings
- [ ] I have added tests that prove my fix is effective or that my feature works
- [ ] New and existing unit tests pass locally with my changes
- [ ] Any dependent changes have been merged and published

## Related Issues
Fixes #(issue number)
Closes #(issue number)
Relates to #(issue number)

## Screenshots/Logs
If applicable, add screenshots or log outputs to help explain your changes.

## Additional Notes
Any additional notes or context about the changes.
