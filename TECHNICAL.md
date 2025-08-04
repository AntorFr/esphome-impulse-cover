# Technical Documentation - Impulse Cover Component

## Architecture Overview

The Impulse Cover component is designed as a state machine that manages single-pulse control systems. It inherits from both `cover::Cover` and `Component` to integrate with ESPHome's cover platform while providing custom timing and safety logic.

## Core Classes

### ImpulseCover
Main component class that handles:
- State management (IDLE, OPENING, CLOSING)
- Position calculation based on time
- Safety monitoring
- Pulse generation and timing

### State Management

The component uses an internal state machine with three primary states:

```cpp
enum class ImpulseCoverOperation {
  IDLE = 0,      // Cover is not moving
  OPENING = 1,   // Cover is moving towards open position  
  CLOSING = 2,   // Cover is moving towards closed position
};
```

## Key Features Implementation

### 1. Single Pulse Logic

The core logic handles different scenarios based on current state:

```cpp
void ImpulseCover::control(const cover::CoverCall &call) {
  if (current_operation_ == ImpulseCoverOperation::IDLE) {
    // Start movement in requested direction
    start_direction(direction);
  } else {
    // Cover is moving - pulse will stop it
    send_pulse();
    stop_movement();
  }
}
```

### 2. Position Calculation

Position is calculated based on elapsed time and configured durations:

```cpp
void ImpulseCover::update_position() {
  float position_change = 0.0f;
  if (current_operation_ == ImpulseCoverOperation::OPENING) {
    position_change = (float) elapsed / open_duration_;
  } else if (current_operation_ == ImpulseCoverOperation::CLOSING) {
    position_change = -(float) elapsed / close_duration_;
  }
  
  position = clamp(position + position_change, 0.0f, 1.0f);
}
```

### 3. Safety System

The component implements multiple safety mechanisms:

#### Timeout Protection
```cpp
if ((now - operation_start_time_) > safety_timeout_) {
  ESP_LOGW(TAG, "Safety timeout triggered");
  safety_triggered_ = true;
  stop_movement();
}
```

#### Cycling Detection
```cpp
if (safety_cycle_count_ >= safety_max_cycles_) {
  ESP_LOGW(TAG, "Safety cycling detected");
  safety_triggered_ = true; 
  stop_movement();
}
```

### 4. Reverse Movement Logic

When reversing direction mid-movement:

1. **First Pulse**: Stops current movement
2. **Delay**: Wait for `pulse_delay_` milliseconds
3. **Second Pulse**: Starts movement in opposite direction

```cpp
if (should_reverse) {
  send_pulse();  // Stop current movement
  pending_reverse_ = true;
  last_pulse_time_ = millis();
  target_operation_ = opposite_direction;
}
```

### 5. Endstop Integration

Optional binary sensors provide accurate position feedback:

```cpp
void ImpulseCover::set_open_sensor(binary_sensor::BinarySensor *sensor) {
  sensor->add_on_state_callback([this](bool state) {
    if (state && current_operation_ == ImpulseCoverOperation::OPENING) {
      position = COVER_OPEN;
      stop_movement();
    }
  });
}
```

## Configuration System

The Python configuration system (`__init__.py`) handles:

- Parameter validation
- Default value assignment  
- Hardware binding (output, sensors)
- Automation trigger setup

### Key Configuration Elements

```python
CONFIG_SCHEMA = cover.COVER_SCHEMA.extend({
  cv.Required(CONF_OUTPUT): cv.use_id(output.BinaryOutput),
  cv.Required(CONF_OPEN_DURATION): cv.positive_time_period_milliseconds,
  cv.Required(CONF_CLOSE_DURATION): cv.positive_time_period_milliseconds,
  cv.Optional(CONF_PULSE_DELAY, default="500ms"): cv.positive_time_period_milliseconds,
  # ... additional options
})
```

## Timing Diagrams

### Normal Operation Sequence

```
Time:    0ms    100ms   15000ms
State:   IDLE → OPENING → IDLE
Pulse:   ■─────────────────────
Position: 0.0 → 0.xx → 1.0
```

### Reverse Operation Sequence

```
Time:    0ms    5000ms  5500ms  6000ms  18000ms
State:   OPENING → IDLE → CLOSING → IDLE
Pulse:   ■─────────────────■─────────■──────────
Action:         Stop     Delay   Reverse
```

### Safety Timeout

```
Time:    0ms              60000ms
State:   OPENING → → → →   IDLE (Safety)
Pulse:   ■─────────────────X (Forced Stop)
```

## Error Handling

### Safety Mode Recovery

Safety mode automatically resets after 30 seconds of idle time:

```cpp
if (safety_triggered_ && current_operation_ == ImpulseCoverOperation::IDLE &&
    (now - operation_start_time_) > 30000) {
  ESP_LOGI(TAG, "Resetting safety trigger");
  safety_triggered_ = false;
  safety_cycle_count_ = 0;
}
```

### Position State Recovery

Position state is persisted across reboots using ESPHome's state restoration:

```cpp
auto restore = this->restore_state_();
if (restore.has_value()) {
  restore->apply(this);
  has_initial_state_ = true;
}
```

## Performance Considerations

### Loop Frequency
- Position updates every 100ms
- Safety checks every loop iteration
- State changes trigger immediate response

### Memory Usage
- Minimal state variables
- No dynamic memory allocation in loop
- Efficient time-based calculations

### CPU Usage
- Simple arithmetic operations
- Minimal logging in normal operation
- Event-driven state changes

## Testing Strategy

### Unit Tests
- Configuration validation
- State machine transitions
- Safety condition triggers
- Position calculations

### Integration Tests  
- Hardware pulse generation
- Sensor integration
- Home Assistant communication
- Error recovery scenarios

### Manual Testing
- Physical movement verification
- Timing accuracy validation
- Safety system verification
- Edge case handling

## Extension Points

### Adding New Features

1. **Custom Triggers**: Extend automation trigger classes
2. **Additional Sensors**: Add new sensor types in configuration
3. **Custom Safety Logic**: Override safety check methods
4. **Position Algorithms**: Alternative position calculation methods

### Hardware Compatibility

The component is designed to work with:
- ESP32/ESP8266 platforms
- Any GPIO-controllable relay/output
- Various sensor types (magnetic, optical, mechanical)
- Different motor controller types

## Debugging

### Log Levels

```cpp
ESP_LOGCONFIG(TAG, "Configuration info");
ESP_LOGD(TAG, "Debug information");  
ESP_LOGI(TAG, "General information");
ESP_LOGW(TAG, "Warning conditions");
ESP_LOGE(TAG, "Error conditions");
```

### Common Debug Steps

1. **Verify Configuration**: Check logs for config validation
2. **Monitor State Changes**: Enable debug logging
3. **Check Timing**: Verify duration calculations
4. **Safety Analysis**: Monitor safety trigger conditions
5. **Hardware Verification**: Test pulse output directly

## Future Enhancements

### Planned Features
- Acceleration/deceleration curves
- Multiple position presets
- Advanced obstruction detection
- Wear monitoring
- Predictive maintenance alerts

### API Extensions
- REST endpoints for direct control
- MQTT integration improvements  
- Enhanced Home Assistant device info
- Mobile app integration hooks
