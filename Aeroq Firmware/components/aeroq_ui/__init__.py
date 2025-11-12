import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_USERNAME, CONF_PASSWORD
from esphome.components import sensor, update, web_server_base

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
CONF_UPDATE = "update"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(AeroqUI),
    cv.Optional(CONF_USERNAME, default=""): cv.string,
    cv.Optional(CONF_PASSWORD, default=""): cv.string,
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
    cv.Required(CONF_UPDATE): cv.use_id(update.UpdateComponent),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    co2   = await cg.get_variable(config[CONF_CO2])
    pm25  = await cg.get_variable(config[CONF_PM25])
    t_scd = await cg.get_variable(config[CONF_T_SCD])
    rh_scd= await cg.get_variable(config[CONF_RH_SCD])
    t_sen = await cg.get_variable(config[CONF_T_SEN])
    rh_sen= await cg.get_variable(config[CONF_RH_SEN])
    pm1   = await cg.get_variable(config[CONF_PM1])
    pm4   = await cg.get_variable(config[CONF_PM4])
    pm10  = await cg.get_variable(config[CONF_PM10])
    voc   = await cg.get_variable(config[CONF_VOC])
    up    = await cg.get_variable(config[CONF_UPDATE])

    cg.add(var.set_sensors(co2, pm25, t_scd, rh_scd, t_sen, rh_sen, pm1, pm4, pm10, voc))
    cg.add(var.set_update(up))
    cg.add(var.set_basic_auth(config[CONF_USERNAME], config[CONF_PASSWORD]))
