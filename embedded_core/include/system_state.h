#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    SYSTEM_STATE_INIT = 0,
    SYSTEM_STATE_SELF_TEST,
    SYSTEM_STATE_NORMAL_OPERATION,
    SYSTEM_STATE_DEGRADED_MODE,
    SYSTEM_STATE_FAIL_SAFE,
    SYSTEM_STATE_MAINTENANCE
} SystemState;

typedef enum
{
    FAULT_LEVEL_NO_FAULT = 0,
    FAULT_LEVEL_MINOR,
    FAULT_LEVEL_MAJOR,
    FAULT_LEVEL_CRITICAL,
    FAULT_LEVEL_CATASTROPHIC
} FaultLevel;

typedef enum
{
    SYSTEM_EVENT_POWER_ON = 0,
    SYSTEM_EVENT_SELF_TEST_PASSED,
    SYSTEM_EVENT_SELF_TEST_FAILED,
    SYSTEM_EVENT_MINOR_FAULT_DETECTED,
    SYSTEM_EVENT_MAJOR_FAULT_DETECTED,
    SYSTEM_EVENT_CRITICAL_FAULT_DETECTED,
    SYSTEM_EVENT_MAINTENANCE_REQUESTED,
    SYSTEM_EVENT_RESET_REQUESTED
} SystemEvent;

typedef struct
{
    SystemState current_state;
    FaultLevel current_fault;
} SystemStateMachine;

void SystemState_Init(SystemStateMachine *machine);

SystemState SystemState_GetCurrentState(const SystemStateMachine *machine);

FaultLevel SystemState_GetCurrentFault(const SystemStateMachine *machine);

int SystemState_HandleEvent(SystemStateMachine *machine, SystemEvent event);

const char *SystemState_ToString(SystemState state);

const char *FaultLevel_ToString(FaultLevel fault);

#ifdef __cplusplus
}
#endif

#endif