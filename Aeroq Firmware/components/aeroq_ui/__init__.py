import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components import sensor, web_server_base

AUTO_LOAD = ["web_server_base"]

aeroq_ns = cg.esphome_ns.namespace("aeroq")
AeroqUI = aeroq_ns.class_("AeroqUI", cg.Component)

CONF_CO2 = "co2"
CONF_PM25 = "pm25"
CONF_T_SCD = "t_scd"
CONF_RH_SCD = "rh_scd"
CONF_T_SEN = "t_sen"
CONF_RH_SEN = "rh_sen"
CONF_PM1 = "pm1"
CONF_PM4 = "pm4"
CONF_PM10 = "pm10"
CONF_VOC = "voc"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_ID): cv.declare_id(AeroqUI),

        cv.Required(CONF_CO2): cv.use_id(sensor.Sensor),
        cv.Required(CONF_PM25): cv.use_id(sensor.Sensor),
        cv.Required(CONF_T_SCD): cv.use_id(sensor.Sensor),
        cv.Required(CONF_RH_SCD): cv.use_id(sensor.Sensor),
        cv.Required(CONF_T_SEN): cv.use_id(sensor.Sensor),
        cv.Required(CONF_RH_SEN): cv.use_id(sensor.Sensor),
        cv.Required(CONF_PM1): cv.use_id(sensor.Sensor),
        cv.Required(CONF_PM4): cv.use_id(sensor.Sensor),
        cv.Required(CONF_PM10): cv.use_id(sensor.Sensor),
        cv.Required(CONF_VOC): cv.use_id(sensor.Sensor),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    # Wire up all sensor pointers → calls set_co2, set_pm25, etc. in C++
    for key in [
        CONF_CO2,
        CONF_PM25,
        CONF_T_SCD,
        CONF_RH_SCD,
        CONF_T_SEN,
        CONF_RH_SEN,
        CONF_PM1,
        CONF_PM4,
        CONF_PM10,
        CONF_VOC,
    ]:
        s = await cg.get_variable(config[key])
        cg.add(getattr(var, f"set_{key}")(s))
