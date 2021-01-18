"""
OBIS object identifiers.
Their only purpose here is to inform the unit conversion.
"""

import datetime


def float_without_unit(x: str) -> float:
    return float(x[:x.index('*')])


def tst(x: str) -> datetime:
    # dst = x[-1] == 'S'  # Not used.
    # 20 prepended to get a full year and avoid any possible ambiguity there.
    return datetime.datetime.strptime("20" + x[:-1], "%Y%m%d%H%M%S").astimezone(datetime.timezone.utc)


DSMR_V5_0_2 = {
    # Electric
    (0, 96,    1,  1):  (str, "id", "Equipment identifier"),
    (1, 1,     8,  1):  (float_without_unit, "meter_t1_used", "Meter Reading electricity delivered to client (tariff 1) in 0,001 kWh"),
    (1, 1,     8,  2):  (float_without_unit, "meter_t2_used", "Meter Reading electricity delivered to client (tariff 2) in 0,001 kWh"),
    (1, 2,     8,  1):  (float_without_unit, "meter_t1_injected", "Meter Reading electricity delivered by client (tariff 1) in 0,001 kWh"),
    (1, 2,     8,  2):  (float_without_unit, "meter_t2_injected", "Meter Reading electricity delivered by client (tariff 2) in 0,001 kWh"),
    (0, 96,    14, 0):  (int, "tariff", "Tariff indicator electricity."),
    (1, 1,     7,  0):  (float_without_unit, "power_used", "Actual electricity power delivered (+P) in 1 Watt resolution"),
    (1, 2,     7,  0):  (float_without_unit, "power_injected", "Actual electricity power received (-P) in 1 Watt resolution"),
    (0, 96,    7,  21): (int, None, "Number of power failures in any phases"),
    (0, 96,    7,  9):  (int, None, "Number of long power failures in any phases"),
    (1, 99,    97, 0):  (str, None, "Power failure event log"),
    (1, 32,    32, 0):  (int, None, "Number of voltage sags in phase L1"),
    (1, 52,    32, 0):  (int, None, "Number of voltage sags in phase L2"),
    (1, 72,    32, 0):  (int, None, "Number of voltage sags in phase L3"),
    (1, 32,    36, 0):  (int, None, "Number of voltage swells in phase L1"),
    (1, 52,    36, 0):  (int, None, "Number of voltage swells in phase L2"),
    (1, 72,    36, 0):  (int, None, "Number of voltage swells in phase L3"),
    (1, 32,    7,  0):  (float_without_unit, "voltage_l1", "Instantaneous voltage L1"),
    (1, 52,    7,  0):  (float_without_unit, "voltage_l2", "Instantaneous voltage L2"),
    (1, 72,    7,  0):  (float_without_unit, "voltage_l3", "Instantaneous voltage L3"),
    (1, 31,    7,  0):  (float_without_unit, "current_l1", "Instantaneous current L1"),
    (1, 51,    7,  0):  (float_without_unit, "current_l2", "Instantaneous current L2"),
    (1, 71,    7,  0):  (float_without_unit, "current_l3", "Instantaneous current L3"),
    (1, 21,    7,  0):  (float_without_unit, "power_l1_pos", "Instantaneous active power L1 (+P)"),
    (1, 41,    7,  0):  (float_without_unit, "power_l2_pos", "Instantaneous active power L2 (+P)"),
    (1, 61,    7,  0):  (float_without_unit, "power_l3_pos", "Instantaneous active power L3 (+P)"),
    (1, 22,    7,  0):  (float_without_unit, "power_l1_neg", "Instantaneous active power L1 (-P)"),
    (1, 42,    7,  0):  (float_without_unit, "power_l2_neg", "Instantaneous active power L2 (-P)"),
    (1, 62,    7,  0):  (float_without_unit, "power_l3_neg", "Instantaneous active power L3 (-P)"),
    # Extra meters (Gas, Water, Heat, Cold, ...)
    (0, 24,    1,  0):  (str, None, "Device-Type"),
    (0, 96,    1,  0):  (str, None, "Equipment identifier"),
    (0, 24,    2,  1):  (float_without_unit, None, "Last 5-minute value"),
    # Other
    (0, 96,    13, 0):  (str, "message", "Text message"),
    (1, 0,     2,  8):  (str, "version", "Version info"),
    (0, 1,     0,  0):  (tst, "time", "Date-time stamp"),
}
EMUCS_V1_4 = {
    **DSMR_V5_0_2,
    # General
    (0, 96,    1,  4):  (str, "version", "Version information"),
    (0, 96,    13, 1):  (str, None, "Consumer message code"),
    # Electric
    (0, 96,    3,  10): (int, "breaker_state", "Breaker state"),
    (0, 17,    0,  0):  (float_without_unit, "limiter", "Limiter threshold"),
    (1, 31,    4,  0):  (float_without_unit, "fuse", "Fuse supervision threshold (L1)"),
    # Gas
    (0, 96,    1,  1):  (str, None, "M-Bus Device ID 2"),
    (0, 24,    4,  0):  (int, None, "Valve state"),
}
