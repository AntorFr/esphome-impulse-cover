# Impulse Cover Component for ESPHome

A custom cover component for ESPHome that handles single-pulse control systems commonly found in gate and garage door controllers.

## Features

- **Single Pulse Control**: Manages covers that use one button/relay for all operations:
  - Open if closed
  - Close if open  
  - Stop if moving
  - Reverse direction after stop
  
- **Time-based Position Tracking**: Calculates position based on configurable open/close durations

- **Optional Endstop Sensors**: Support for open/closed position sensors for accurate state detection

- **Safety Features**:
  - Movement timeout protection
  - Cycling detection (prevents infinite open/close loops)
  - Automatic safety mode with Home Assistant notifications

- **Reversible Movement**: Can reverse direction mid-movement with configurable pulse delays

- **Home Assistant Integration**: 
  - Position reporting (0-100%)
  - Position setting
  - Standard cover controls (open/close/stop/toggle)

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
