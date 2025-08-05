"""ESPHome Impulse Cover Component."""

import esphome.codegen as cg
from esphome.components import cover
import esphome.config_validation as cv

DEPENDENCIES = ["cover"]

# Component namespace
impulse_cover_ns = cg.esphome_ns.namespace("impulse_cover")


# Empty to_code function - the actual implementation is in cover.py
async def to_code(config):  # pylint: disable=unused-argument
    """Empty to_code function - implementation in cover.py."""
