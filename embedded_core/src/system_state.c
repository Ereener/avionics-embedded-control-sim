#include "system_state.h"

static int is_valid_transition(SystemState current_state, SystemEvent event)
{
    switch (current_state)
    {
    case SYSTEM_STATE_INIT:
        return event == SYSTEM_EVENT_POWER_ON;

    case SYSTEM_STATE_SELF_TEST:
        return event == SYSTEM_EVENT_SELF_TEST_PASSED ||
               event == SYSTEM_EVENT_SELF_TEST_FAILED;

    case SYSTEM_STATE_NORMAL_OPERATION:
        return event == SYSTEM_EVENT_MINOR_FAULT_DETECTED ||
               event == SYSTEM_EVENT_MAJOR_FAULT_DETECTED ||
               event == SYSTEM_EVENT_CRITICAL_FAULT_DETECTED ||
               event == SYSTEM_EVENT_MAINTENANCE_REQUESTED;

    case SYSTEM_STATE_DEGRADED_MODE:
        return event == SYSTEM_EVENT_MAJOR_FAULT_DETECTED ||
               event == SYSTEM_EVENT_CRITICAL_FAULT_DETECTED ||
               event == SYSTEM_EVENT_MAINTENANCE_REQUESTED ||
               event == SYSTEM_EVENT_RESET_REQUESTED;

    case SYSTEM_STATE_FAIL_SAFE:
        return event == SYSTEM_EVENT_MAINTENANCE_REQUESTED ||
               event == SYSTEM_EVENT_RESET_REQUESTED;

    case SYSTEM_STATE_MAINTENANCE:
        return event == SYSTEM_EVENT_RESET_REQUESTED;

    default:
        return 0;
    }
}

void SystemState_Init(SystemStateMachine *machine)
{
    if (machine == 0)
    {
        return;
    }

    machine->current_state = SYSTEM_STATE_INIT;
    machine->current_fault = FAULT_LEVEL_NO_FAULT;
}

SystemState SystemState_GetCurrentState(const SystemStateMachine *machine)
{
    if (machine == 0)
    {
        return SYSTEM_STATE_FAIL_SAFE;
    }

    return machine->current_state;
}

FaultLevel SystemState_GetCurrentFault(const SystemStateMachine *machine)
{
    if (machine == 0)
    {
        return FAULT_LEVEL_CATASTROPHIC;
    }

    return machine->current_fault;
}

int SystemState_HandleEvent(SystemStateMachine *machine, SystemEvent event)
{
    if (machine == 0)
    {
        return 0;
    }

    if (!is_valid_transition(machine->current_state, event))
    {
        return 0;
    }

    switch (machine->current_state)
    {
    case SYSTEM_STATE_INIT:
        if (event == SYSTEM_EVENT_POWER_ON)
        {
            machine->current_state = SYSTEM_STATE_SELF_TEST;
        }
        break;

    case SYSTEM_STATE_SELF_TEST:
        if (event == SYSTEM_EVENT_SELF_TEST_PASSED)
        {
            machine->current_state = SYSTEM_STATE_NORMAL_OPERATION;
            machine->current_fault = FAULT_LEVEL_NO_FAULT;
        }
        else if (event == SYSTEM_EVENT_SELF_TEST_FAILED)
        {
            machine->current_state = SYSTEM_STATE_FAIL_SAFE;
            machine->current_fault = FAULT_LEVEL_CRITICAL;
        }
        break;

    case SYSTEM_STATE_NORMAL_OPERATION:
        if (event == SYSTEM_EVENT_MINOR_FAULT_DETECTED)
        {
            machine->current_state = SYSTEM_STATE_DEGRADED_MODE;
            machine->current_fault = FAULT_LEVEL_MINOR;
        }
        else if (event == SYSTEM_EVENT_MAJOR_FAULT_DETECTED)
        {
            machine->current_state = SYSTEM_STATE_DEGRADED_MODE;
            machine->current_fault = FAULT_LEVEL_MAJOR;
        }
        else if (event == SYSTEM_EVENT_CRITICAL_FAULT_DETECTED)
        {
            machine->current_state = SYSTEM_STATE_FAIL_SAFE;
            machine->current_fault = FAULT_LEVEL_CRITICAL;
        }
        else if (event == SYSTEM_EVENT_MAINTENANCE_REQUESTED)
        {
            machine->current_state = SYSTEM_STATE_MAINTENANCE;
        }
        break;

    case SYSTEM_STATE_DEGRADED_MODE:
        if (event == SYSTEM_EVENT_MAJOR_FAULT_DETECTED)
        {
            machine->current_fault = FAULT_LEVEL_MAJOR;
        }
        else if (event == SYSTEM_EVENT_CRITICAL_FAULT_DETECTED)
        {
            machine->current_state = SYSTEM_STATE_FAIL_SAFE;
            machine->current_fault = FAULT_LEVEL_CRITICAL;
        }
        else if (event == SYSTEM_EVENT_MAINTENANCE_REQUESTED)
        {
            machine->current_state = SYSTEM_STATE_MAINTENANCE;
        }
        else if (event == SYSTEM_EVENT_RESET_REQUESTED)
        {
            machine->current_state = SYSTEM_STATE_INIT;
            machine->current_fault = FAULT_LEVEL_NO_FAULT;
        }
        break;

    case SYSTEM_STATE_FAIL_SAFE:
        if (event == SYSTEM_EVENT_MAINTENANCE_REQUESTED)
        {
            machine->current_state = SYSTEM_STATE_MAINTENANCE;
        }
        else if (event == SYSTEM_EVENT_RESET_REQUESTED)
        {
            machine->current_state = SYSTEM_STATE_INIT;
            machine->current_fault = FAULT_LEVEL_NO_FAULT;
        }
        break;

    case SYSTEM_STATE_MAINTENANCE:
        if (event == SYSTEM_EVENT_RESET_REQUESTED)
        {
            machine->current_state = SYSTEM_STATE_INIT;
            machine->current_fault = FAULT_LEVEL_NO_FAULT;
        }
        break;

    default:
        machine->current_state = SYSTEM_STATE_FAIL_SAFE;
        machine->current_fault = FAULT_LEVEL_CATASTROPHIC;
        return 0;
    }

    return 1;
}

const char *SystemState_ToString(SystemState state)
{
    switch (state)
    {
    case SYSTEM_STATE_INIT:
        return "INIT";
    case SYSTEM_STATE_SELF_TEST:
        return "SELF_TEST";
    case SYSTEM_STATE_NORMAL_OPERATION:
        return "NORMAL_OPERATION";
    case SYSTEM_STATE_DEGRADED_MODE:
        return "DEGRADED_MODE";
    case SYSTEM_STATE_FAIL_SAFE:
        return "FAIL_SAFE";
    case SYSTEM_STATE_MAINTENANCE:
        return "MAINTENANCE";
    default:
        return "UNKNOWN_STATE";
    }
}

const char *FaultLevel_ToString(FaultLevel fault)
{
    switch (fault)
    {
    case FAULT_LEVEL_NO_FAULT:
        return "NO_FAULT";
    case FAULT_LEVEL_MINOR:
        return "MINOR";
    case FAULT_LEVEL_MAJOR:
        return "MAJOR";
    case FAULT_LEVEL_CRITICAL:
        return "CRITICAL";
    case FAULT_LEVEL_CATASTROPHIC:
        return "CATASTROPHIC";
    default:
        return "UNKNOWN_FAULT";
    }
}