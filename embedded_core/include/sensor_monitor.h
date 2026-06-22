#ifndef SENSOR_MONITOR_H
#define SENSOR_MONITOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "system_state.h"

typedef enum
{
    SENSOR_STATUS_OK = 0,
    SENSOR_STATUS_WARNING,
    SENSOR_STATUS_OUT_OF_RANGE,
    SENSOR_STATUS_INVALID
} SensorStatus;

typedef struct
{
    uint16_t raw_adc_value;
    float engineering_value;
    SensorStatus status;
} SensorReading;

typedef struct
{
    SensorReading temperature_celsius;
    SensorReading voltage_volts;
    SensorReading actuator_position_percent;
    FaultLevel overall_fault_level;
} SensorMonitor;

void SensorMonitor_Init(SensorMonitor *monitor);

void SensorMonitor_UpdateTemperature(SensorMonitor *monitor, uint16_t raw_adc_value);

void SensorMonitor_UpdateVoltage(SensorMonitor *monitor, uint16_t raw_adc_value);

void SensorMonitor_UpdateActuatorPosition(SensorMonitor *monitor, uint16_t raw_adc_value);

FaultLevel SensorMonitor_GetOverallFaultLevel(const SensorMonitor *monitor);

const char *SensorStatus_ToString(SensorStatus status);

#ifdef __cplusplus
}
#endif

#endif