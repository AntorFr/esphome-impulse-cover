# Impulse Cover Component for ESPHome

A custom cover component for ESPHome that handles single-pulse control systems commonly found in gate and garage door controllers.

## Installation

Add this component to your ESPHome configuration:

```yaml
external_components:
  - source: github://AntorFr/esphome-impulse-cover
    components: [ impulse_cover ]
```

## Features

- **Single Pulse Control**: Manages covers that use one button/relay for all operations:
  - Open if closed
  - Close if open  
  - Stop if moving
  - Reverse direction after stop
  
- **Precise Position Control**: 
  - Configurable open/close durations for accurate position calculation
  - **Partial Opening Support**: Set any position from 0% to 100%
  - Real-time position tracking during movement
  - Position memory between operations

- **Time-based Movement**: Calculates position based on configurable open/close durations with automatic progression from current position to target

- **Optional Endstop Sensors**: Support for open/closed position sensors for accurate state detection and automatic position correction

- **Safety Features**:
  - Movement timeout protection
  - Cycling detection (prevents infinite open/close loops)
  - Automatic safety mode with Home Assistant notifications

- **Reversible Movement**: Can reverse direction mid-movement with configurable pulse delays

- **Home Assistant Integration**: 
  - Position reporting (0-100%)
  - **Position setting (partial opening)**
  - Standard cover controls (open/close/stop/toggle)
  - Web server interface for testing and control

## Configuration

```yaml
external_components:
  - source: github://yourusername/esphome-impulse-cover
    components: [ impulse_cover ]

# Define the output for pulses
output:
  - platform: gpio
    pin: GPIO2
    id: gate_output

# Optional endstop sensors
binary_sensor:
  - platform: gpio
    pin: GPIO4
    name: "Gate Open Sensor"
    id: gate_open_sensor
    
  - platform: gpio
    pin: GPIO5
    name: "Gate Close Sensor" 
    id: gate_close_sensor

cover:
  - platform: impulse_cover
    name: "Main Gate"
    output: gate_output
    
    # Required: Movement durations
    open_duration: 15s
    close_duration: 15s
    
    # Optional: Safety and timing settings
    pulse_delay: 500ms          # Delay between stop and reverse pulses
    safety_timeout: 60s         # Max time for any operation
    safety_max_cycles: 5        # Max direction changes before safety trigger
    
    # Optional: Endstop sensors
    open_sensor: gate_open_sensor
    close_sensor: gate_close_sensor
    
    # Optional: Sensor logic inversion (for different sensor types)
    open_sensor_inverted: false     # Set to true for active LOW sensors  
    close_sensor_inverted: false    # Set to true for active LOW sensors
    
    # Optional: Automation triggers
    on_open:
      - logger.log: "Gate is opening"
    on_close:
      - logger.log: "Gate is closing"
    on_idle:
      - logger.log: "Gate stopped"
    on_safety:
      - logger.log: 
          format: "Gate safety triggered!"
          level: WARN
      - homeassistant.event:
          event: esphome.gate_safety_alert
          data:
            device: "main_gate"
            message: "Safety mode activated"
```

### Endstop Sensors

Optional binary sensors can be configured to detect when the cover reaches its fully open or closed positions:

#### Sensor Types
- **Reed switches** with magnets
- **Limit switches** (mechanical)
- **Optical sensors** (photoelectric)
- **Hall effect sensors**

#### Sensor Logic
Different sensors have different logic levels:
- **Active HIGH**: Sensor outputs HIGH (3.3V) when triggered
- **Active LOW**: Sensor outputs LOW (0V) when triggered  

Use the inversion options to match your sensor type:
```yaml
cover:
  - platform: impulse_cover
    # ... other config
    open_sensor: my_open_sensor
    close_sensor: my_close_sensor
    open_sensor_inverted: true    # For active LOW open sensor
    close_sensor_inverted: false  # For active HIGH close sensor
```

## How It Works

### Single Pulse Logic

The component sends a single pulse to the output which causes different behaviors based on the current state:

1. **Idle + Closed**: Pulse → Start Opening
2. **Idle + Open**: Pulse → Start Closing  
3. **Moving**: Pulse → Stop
4. **Stopped Mid-Movement**: Pulse → Resume in opposite direction

### Position Calculation

- Position is calculated based on elapsed time and configured durations
- Endstop sensors provide accurate position correction when available
- Position is persisted across reboots for stateful operation

### Safety Features

1. **Timeout Protection**: If movement exceeds the safety timeout, operation stops
2. **Cycling Detection**: If too many direction changes occur rapidly, safety mode activates
3. **Safety Recovery**: Safety mode automatically resets after extended idle period

### Reverse Movement

When changing direction mid-movement:
1. First pulse stops current movement
2. Wait for `pulse_delay` duration
3. Second pulse starts movement in opposite direction

## Partial Opening Usage

The component supports precise partial opening control:

### Via Home Assistant

```yaml
# Set specific positions
service: cover.set_cover_position
target:
  entity_id: cover.main_gate
data:
  position: 30  # 30% open for pedestrian access

# Or use automation
automation:
  - id: pedestrian_access
    trigger:
      - platform: state
        entity_id: binary_sensor.pedestrian_button
        to: 'on'
    action:
      - cover.control:
          id: main_gate
          position: 0.3  # 30% open
```

### Common Use Cases

```yaml
# Pedestrian access (30%)
position: 0.3

# Small vehicle access (60%) 
position: 0.6

# Full vehicle access (100%)
position: 1.0

# Fully closed
position: 0.0
```

### Position Calculation

The component automatically calculates movement:
- **From current position to target position**
- **Proportional timing**: 50% movement = 50% of full duration
- **Real-time updates**: Position updated during movement
- **Auto-correction**: Endstop sensors provide precise positioning

Example: Gate at 20% wants to reach 70%
- Distance: 50% of full range
- Time needed: `open_duration × 0.5`
- Position updates in real-time during movement

## API Reference

### Configuration Options

| Option | Type | Default | Description |
|--------|------|---------|-------------|
| `output` | Output | Required | GPIO output for sending pulses |
| `open_duration` | Time | Required | Time to fully open |
| `close_duration` | Time | Required | Time to fully close |
| `pulse_delay` | Time | 500ms | Delay between stop and reverse pulses |
| `safety_timeout` | Time | 60s | Maximum operation time |
| `safety_max_cycles` | Integer | 5 | Max cycles before safety trigger |
| `open_sensor` | Binary Sensor | Optional | Sensor for open position |
| `close_sensor` | Binary Sensor | Optional | Sensor for closed position |
| `open_sensor_inverted` | Boolean | false | Invert open sensor logic (for active LOW) |
| `close_sensor_inverted` | Boolean | false | Invert close sensor logic (for active LOW) |

### Automation Triggers

- `on_open`: Triggered when opening starts
- `on_close`: Triggered when closing starts  
- `on_idle`: Triggered when movement stops
- `on_safety`: Triggered when safety mode activates

## Use Cases

This component is ideal for:

- **Gate Controllers**: Single-button gate systems
- **Garage Doors**: Simple garage door controllers
- **Industrial Doors**: Basic industrial door systems
- **Shutters**: Motor-driven shutters with simple control

## Documentation

- **[Partial Opening Guide](docs/PARTIAL_OPENING.md)**: Detailed configuration and usage for partial opening functionality
- **[Examples](examples/)**: Complete configuration examples
  - `basic-configuration.yaml`: Minimal setup without sensors
  - `with-sensors.yaml`: Full setup with endstop sensors
  - `partial-test.yaml`: Configuration optimized for partial opening

## Testing

Use the included test script to validate your setup:

```bash
# Quick validation test
./test-impulse-cover.sh

# Full validation + compilation test
./test-impulse-cover.sh --compile
```

See **[Test Script Documentation](docs/TEST_SCRIPT.md)** for detailed usage.

## Troubleshooting

### Cover doesn't respond
- Check output GPIO configuration
- Verify wiring to control system
- Check ESPHome logs for errors

### Position inaccurate
- Calibrate `open_duration` and `close_duration` values
- Add endstop sensors for accurate positioning
- Check for mechanical issues affecting timing

### Safety mode activating
- Check for sensor issues causing false triggers
- Adjust `safety_max_cycles` if needed
- Verify mechanical operation is smooth

### Reverse operation not working
- Increase `pulse_delay` if pulses are too close together
- Check that the controlled system supports stop/reverse logic

## Contributing

Issues and pull requests are welcome! Please follow ESPHome coding standards and include tests for new features.

## License

This component is released under the MIT License.
