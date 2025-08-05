import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover, binary_sensor, output
from esphome.const import (
    CONF_ID,
    CONF_OPEN_DURATION,
    CONF_CLOSE_DURATION,
    CONF_OUTPUT,
)

DEPENDENCIES = ["cover"]

# Constants for configuration
CONF_PULSE_DELAY = "pulse_delay"
CONF_SAFETY_TIMEOUT = "safety_timeout"
CONF_SAFETY_MAX_CYCLES = "safety_max_cycles"
CONF_OPEN_SENSOR = "open_sensor"
CONF_CLOSE_SENSOR = "close_sensor"
CONF_OPEN_SENSOR_INVERTED = "open_sensor_inverted"
CONF_CLOSE_SENSOR_INVERTED = "close_sensor_inverted"

# Component namespace and class
impulse_cover_ns = cg.esphome_ns.namespace("impulse_cover")
ImpulseCover = impulse_cover_ns.class_("ImpulseCover", cover.Cover, cg.Component)

CONFIG_SCHEMA = cover.cover_schema(ImpulseCover).extend({
    cv.Required(CONF_OUTPUT): cv.use_id(output.BinaryOutput),
    cv.Required(CONF_OPEN_DURATION): cv.positive_time_period_milliseconds,
    cv.Required(CONF_CLOSE_DURATION): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_PULSE_DELAY, default="500ms"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_SAFETY_TIMEOUT, default="60s"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_SAFETY_MAX_CYCLES, default=5): cv.int_range(min=1, max=20),
    cv.Optional(CONF_OPEN_SENSOR): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional(CONF_CLOSE_SENSOR): cv.use_id(binary_sensor.BinarySensor),
    cv.Optional(CONF_OPEN_SENSOR_INVERTED, default=False): cv.boolean,
    cv.Optional(CONF_CLOSE_SENSOR_INVERTED, default=False): cv.boolean,
}).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)

    # Set required parameters
    cg.add(var.set_open_duration(config[CONF_OPEN_DURATION]))
    cg.add(var.set_close_duration(config[CONF_CLOSE_DURATION]))
    
    # Set optional parameters
    cg.add(var.set_pulse_delay(config[CONF_PULSE_DELAY]))
    cg.add(var.set_safety_timeout(config[CONF_SAFETY_TIMEOUT]))
    cg.add(var.set_safety_max_cycles(config[CONF_SAFETY_MAX_CYCLES]))

    # Set output
    output_var = await cg.get_variable(config[CONF_OUTPUT])
    cg.add(var.set_output(output_var))

    # Set sensors if provided
    if CONF_OPEN_SENSOR in config:
        open_sensor = await cg.get_variable(config[CONF_OPEN_SENSOR])
        cg.add(var.set_open_sensor(open_sensor))
        cg.add(var.set_open_sensor_inverted(config[CONF_OPEN_SENSOR_INVERTED]))

    if CONF_CLOSE_SENSOR in config:
        close_sensor = await cg.get_variable(config[CONF_CLOSE_SENSOR])
        cg.add(var.set_close_sensor(close_sensor))
        cg.add(var.set_close_sensor_inverted(config[CONF_CLOSE_SENSOR_INVERTED]))
