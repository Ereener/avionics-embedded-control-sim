#include "sensor_monitor.h"

#define ADC_MAX_VALUE             4095.0f

#define TEMP_MIN_CELSIUS          -40.0f
#define TEMP_WARNING_CELSIUS      85.0f
#define TEMP_CRITICAL_CELSIUS     105.0f
#define TEMP_MAX_CELSIUS          125.0f

#define VOLTAGE_MIN_VOLTS         18.0f
#define VOLTAGE_WARNING_LOW       22.0f
#define VOLTAGE_NOMINAL_MAX       29.0f
#define VOLTAGE_WARNING_HIGH      32.0f
#define VOLTAGE_MAX_VOLTS         36.0f

#define ACTUATOR_MIN_PERCENT      0.0f
#define ACTUATOR_MAX_PERCENT      100.0f

static float adc_to_temperature(uint16_t raw_adc_value)
{
    return TEMP_MIN_CELSIUS +
           ((float)raw_adc_value / ADC_MAX_VALUE) *
           (TEMP_MAX_CELSIUS - TEMP_MIN_CELSIUS);
}

static float adc_to_voltage(uint16_t raw_adc_value)
{
    return ((float)raw_adc_value / ADC_MAX_VALUE) * VOLTAGE_MAX_VOLTS;
}

static float adc_to_actuator_position(uint16_t raw_adc_value)
{
    return ((float)raw_adc_value / ADC_MAX_VALUE) * ACTUATOR_MAX_PERCENT;
}

static SensorStatus evaluate_temperature(float temperature)
{
    if (temperature < TEMP_MIN_CELSIUS || temperature > TEMP_MAX_CELSIUS)
    {
        return SENSOR_STATUS_INVALID;
    }

    if (temperature >= TEMP_CRITICAL_CELSIUS)
    {
        return SENSOR_STATUS_OUT_OF_RANGE;
    }

    if (temperature >= TEMP_WARNING_CELSIUS)
    {
        return SENSOR_STATUS_WARNING;
    }

    return SENSOR_STATUS_OK;
}

static SensorStatus evaluate_voltage(float voltage)
{
    if (voltage < VOLTAGE_MIN_VOLTS || voltage > VOLTAGE_MAX_VOLTS)
    {
        return SENSOR_STATUS_OUT_OF_RANGE;
    }

    if (voltage < VOLTAGE_WARNING_LOW || voltage > VOLTAGE_NOMINAL_MAX)
    {
        return SENSOR_STATUS_WARNING;
    }

    return SENSOR_STATUS_OK;
}

static SensorStatus evaluate_actuator_position(float position)
{
    if (position < ACTUATOR_MIN_PERCENT || position > ACTUATOR_MAX_PERCENT)
    {
        return SENSOR_STATUS_INVALID;
    }

    return SENSOR_STATUS_OK;
}

static FaultLevel map_sensor_status_to_fault(SensorStatus status)
{
    switch (status)
    {
    case SENSOR_STATUS_OK:
        return FAULT_LEVEL_NO_FAULT;

    case SENSOR_STATUS_WARNING:
        return FAULT_LEVEL_MINOR;

    case SENSOR_STATUS_OUT_OF_RANGE:
        return FAULT_LEVEL_MAJOR;

    case SENSOR_STATUS_INVALID:
        return FAULT_LEVEL_CRITICAL;

    default:
        return FAULT_LEVEL_CATASTROPHIC;
    }
}

static FaultLevel max_fault_level(FaultLevel a, FaultLevel b)
{
    return (a > b) ? a : b;
}

static void recalculate_overall_fault_level(SensorMonitor *monitor)
{
    FaultLevel temperature_fault =
        map_sensor_status_to_fault(monitor->temperature_celsius.status);

    FaultLevel voltage_fault =
        map_sensor_status_to_fault(monitor->voltage_volts.status);

    FaultLevel actuator_fault =
        map_sensor_status_to_fault(monitor->actuator_position_percent.status);

    FaultLevel overall = FAULT_LEVEL_NO_FAULT;

    overall = max_fault_level(overall, temperature_fault);
    overall = max_fault_level(overall, voltage_fault);
    overall = max_fault_level(overall, actuator_fault);

    monitor->overall_fault_level = overall;
}

void SensorMonitor_Init(SensorMonitor *monitor)
{
    if (monitor == 0)
    {
        return;
    }

    monitor->temperature_celsius.raw_adc_value = 0;
    monitor->temperature_celsius.engineering_value = 0.0f;
    monitor->temperature_celsius.status = SENSOR_STATUS_OK;

    monitor->voltage_volts.raw_adc_value = 0;
    monitor->voltage_volts.engineering_value = 0.0f;
    monitor->voltage_volts.status = SENSOR_STATUS_OK;

    monitor->actuator_position_percent.raw_adc_value = 0;
    monitor->actuator_position_percent.engineering_value = 0.0f;
    monitor->actuator_position_percent.status = SENSOR_STATUS_OK;

    monitor->overall_fault_level = FAULT_LEVEL_NO_FAULT;
}

void SensorMonitor_UpdateTemperature(SensorMonitor *monitor, uint16_t raw_adc_value)
{
    if (monitor == 0)
    {
        return;
    }

    monitor->temperature_celsius.raw_adc_value = raw_adc_value;
    monitor->temperature_celsius.engineering_value = adc_to_temperature(raw_adc_value);
    monitor->temperature_celsius.status =
        evaluate_temperature(monitor->temperature_celsius.engineering_value);

    recalculate_overall_fault_level(monitor);
}

void SensorMonitor_UpdateVoltage(SensorMonitor *monitor, uint16_t raw_adc_value)
{
    if (monitor == 0)
    {
        return;
    }

    monitor->voltage_volts.raw_adc_value = raw_adc_value;
    monitor->voltage_volts.engineering_value = adc_to_voltage(raw_adc_value);
    monitor->voltage_volts.status =
        evaluate_voltage(monitor->voltage_volts.engineering_value);

    recalculate_overall_fault_level(monitor);
}

void SensorMonitor_UpdateActuatorPosition(SensorMonitor *monitor, uint16_t raw_adc_value)
{
    if (monitor == 0)
    {
        return;
    }

    monitor->actuator_position_percent.raw_adc_value = raw_adc_value;
    monitor->actuator_position_percent.engineering_value =
        adc_to_actuator_position(raw_adc_value);

    monitor->actuator_position_percent.status =
        evaluate_actuator_position(monitor->actuator_position_percent.engineering_value);

    recalculate_overall_fault_level(monitor);
}

FaultLevel SensorMonitor_GetOverallFaultLevel(const SensorMonitor *monitor)
{
    if (monitor == 0)
    {
        return FAULT_LEVEL_CATASTROPHIC;
    }

    return monitor->overall_fault_level;
}

const char *SensorStatus_ToString(SensorStatus status)
{
    switch (status)
    {
    case SENSOR_STATUS_OK:
        return "OK";

    case SENSOR_STATUS_WARNING:
        return "WARNING";

    case SENSOR_STATUS_OUT_OF_RANGE:
        return "OUT_OF_RANGE";

    case SENSOR_STATUS_INVALID:
        return "INVALID";

    default:
        return "UNKNOWN_SENSOR_STATUS";
    }
}