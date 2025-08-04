import esphome.codegen as cg
from esphome.components import cover

DEPENDENCIES = ["cover"]

CONF_PULSE_DELAY = "pulse_delay"
CONF_SAFETY_TIMEOUT = "safety_timeout"
CONF_SAFETY_MAX_CYCLES = "safety_max_cycles"
CONF_OPEN_SENSOR = "open_sensor"
CONF_CLOSE_SENSOR = "close_sensor"

impulse_cover_ns = cg.esphome_ns.namespace("impulse_cover")
ImpulseCover = impulse_cover_ns.class_("ImpulseCover", cover.Cover, cg.Component)
