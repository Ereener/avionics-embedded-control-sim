#ifndef FAULT_MANAGER_H
#define FAULT_MANAGER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "system_state.h"

#define FAULT_MANAGER_MAX_ACTIVE_FAULTS 16

typedef enum
{
    FAULT_CODE_NONE = 0,
    FAULT_CODE_SENSOR_TEMPERATURE_HIGH,
    FAULT_CODE_SENSOR_VOLTAGE_LOW,
    FAULT_CODE_SENSOR_VOLTAGE_HIGH,
    FAULT_CODE_ACTUATOR_POSITION_INVALID,
    FAULT_CODE_COMMUNICATION_TIMEOUT,
    FAULT_CODE_WATCHDOG_EXPIRED,
    FAULT_CODE_SELF_TEST_FAILED,
    FAULT_CODE_UNKNOWN
} FaultCode;

typedef struct
{
    FaultCode code;
    FaultLevel level;
    uint8_t is_active;
    uint32_t occurrence_count;
} FaultRecord;

typedef struct
{
    FaultRecord active_faults[FAULT_MANAGER_MAX_ACTIVE_FAULTS];
    uint8_t active_fault_count;
    FaultLevel highest_fault_level;
} FaultManager;

void FaultManager_Init(FaultManager *manager);

int FaultManager_ReportFault(FaultManager *manager, FaultCode code, FaultLevel level);

int FaultManager_ClearFault(FaultManager *manager, FaultCode code);

uint8_t FaultManager_IsFaultActive(const FaultManager *manager, FaultCode code);

FaultLevel FaultManager_GetHighestFaultLevel(const FaultManager *manager);

uint8_t FaultManager_GetActiveFaultCount(const FaultManager *manager);

SystemEvent FaultManager_MapFaultLevelToSystemEvent(FaultLevel level);

const char *FaultCode_ToString(FaultCode code);

#ifdef __cplusplus
}
#endif

#endif