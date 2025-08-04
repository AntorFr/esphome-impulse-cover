import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import cover, binary_sensor, output
from esphome.const import (
    CONF_ID,
    CONF_OPEN_DURATION,
    CONF_CLOSE_DURATION,
    CONF_OUTPUT,
)

DEPENDENCIES = ["cover"]

CONF_PULSE_DELAY = "pulse_delay"
CONF_SAFETY_TIMEOUT = "safety_timeout"
CONF_SAFETY_MAX_CYCLES = "safety_max_cycles"
CONF_OPEN_SENSOR = "open_sensor"
CONF_CLOSE_SENSOR = "close_sensor"
CONF_ON_OPEN = "on_open"
CONF_ON_CLOSE = "on_close"
CONF_ON_IDLE = "on_idle"
CONF_ON_SAFETY = "on_safety"

impulse_cover_ns = cg.esphome_ns.namespace("impulse_cover")
ImpulseCover = impulse_cover_ns.class_("ImpulseCover", cover.Cover, cg.Component)

# Triggers
ImpulseCoverOpenTrigger = impulse_cover_ns.class_(
    "ImpulseCoverOpenTrigger", automation.Trigger.template()
)
ImpulseCoverCloseTrigger = impulse_cover_ns.class_(
    "ImpulseCoverCloseTrigger", automation.Trigger.template()
)
ImpulseCoverIdleTrigger = impulse_cover_ns.class_(
    "ImpulseCoverIdleTrigger", automation.Trigger.template()
)
ImpulseCoverSafetyTrigger = impulse_cover_ns.class_(
    "ImpulseCoverSafetyTrigger", automation.Trigger.template()
)

CONFIG_SCHEMA = (
    cover.COVER_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(ImpulseCover),
            cv.Required(CONF_OUTPUT): cv.use_id(output.BinaryOutput),
            cv.Required(CONF_OPEN_DURATION): cv.positive_time_period_milliseconds,
            cv.Required(CONF_CLOSE_DURATION): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_PULSE_DELAY, default="500ms"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_SAFETY_TIMEOUT, default="60s"): cv.positive_time_period_milliseconds,
            cv.Optional(CONF_SAFETY_MAX_CYCLES, default=5): cv.int_range(min=1, max=20),
            cv.Optional(CONF_OPEN_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_CLOSE_SENSOR): cv.use_id(binary_sensor.BinarySensor),
            cv.Optional(CONF_ON_OPEN): automation.validate_automation(
                {
                    cv.GenerateID(automation.TRIGGER_ID): cv.declare_id(
                        ImpulseCoverOpenTrigger
                    ),
                }
            ),
            cv.Optional(CONF_ON_CLOSE): automation.validate_automation(
                {
                    cv.GenerateID(automation.TRIGGER_ID): cv.declare_id(
                        ImpulseCoverCloseTrigger
                    ),
                }
            ),
            cv.Optional(CONF_ON_IDLE): automation.validate_automation(
                {
                    cv.GenerateID(automation.TRIGGER_ID): cv.declare_id(
                        ImpulseCoverIdleTrigger
                    ),
                }
            ),
            cv.Optional(CONF_ON_SAFETY): automation.validate_automation(
                {
                    cv.GenerateID(automation.TRIGGER_ID): cv.declare_id(
                        ImpulseCoverSafetyTrigger
                    ),
                }
            ),
        }
    )
    .extend(cv.COMPONENT_SCHEMA)
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

    # Setup automation triggers
    for conf in config.get(CONF_ON_OPEN, []):
        trigger = cg.new_Pvariable(conf[automation.TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)

    for conf in config.get(CONF_ON_CLOSE, []):
        trigger = cg.new_Pvariable(conf[automation.TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)

    for conf in config.get(CONF_ON_IDLE, []):
        trigger = cg.new_Pvariable(conf[automation.TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)

    for conf in config.get(CONF_ON_SAFETY, []):
        trigger = cg.new_Pvariable(conf[automation.TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)
