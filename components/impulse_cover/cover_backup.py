import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import cover, binary_sensor, output
from esphome.const import (
    CONF_ID,
    CONF_OPEN_DURATION,
    CONF_CLOSE_DURATION,
    CONF_OUTPUT,
)

from . import (
    impulse_cover_ns,
    ImpulseCover,
    CONF_PULSE_DELAY,
    CONF_SAFETY_TIMEOUT,
    CONF_SAFETY_MAX_CYCLES,
    CONF_OPEN_SENSOR,
    CONF_CLOSE_SENSOR,
)

CONFIG_SCHEMA = cv.All(
    cv.Schema({
        cv.GenerateID(): cv.declare_id(ImpulseCover),
        cv.Required(CONF_OUTPUT): cv.use_id(output.BinaryOutput),
        cv.Required(CONF_OPEN_DURATION): cv.positive_time_period_milliseconds,
        cv.Required(CONF_CLOSE_DURATION): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_PULSE_DELAY, default="500ms"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_SAFETY_TIMEOUT, default="60s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_SAFETY_MAX_CYCLES, default=5): cv.int_range(min=1, max=20),
        cv.Optional(CONF_OPEN_SENSOR): cv.use_id(binary_sensor.BinarySensor),
        cv.Optional(CONF_CLOSE_SENSOR): cv.use_id(binary_sensor.BinarySensor),
    }).extend(cv.COMPONENT_SCHEMA).extend(cover.COVER_SCHEMA)
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await cover.register_cover(var, config)

    # Set durations
    cg.add(var.set_open_duration(config[CONF_OPEN_DURATION]))
    cg.add(var.set_close_duration(config[CONF_CLOSE_DURATION]))
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

    if CONF_CLOSE_SENSOR in config:
        close_sensor = await cg.get_variable(config[CONF_CLOSE_SENSOR])
        cg.add(var.set_close_sensor(close_sensor))