#include "fault_manager.h"

static int find_fault_index(const FaultManager *manager, FaultCode code)
{
    uint8_t i;

    if (manager == 0)
    {
        return -1;
    }

    for (i = 0; i < FAULT_MANAGER_MAX_ACTIVE_FAULTS; ++i)
    {
        if (manager->active_faults[i].is_active &&
            manager->active_faults[i].code == code)
        {
            return i;
        }
    }

    return -1;
}

static int find_free_slot(const FaultManager *manager)
{
    uint8_t i;

    if (manager == 0)
    {
        return -1;
    }

    for (i = 0; i < FAULT_MANAGER_MAX_ACTIVE_FAULTS; ++i)
    {
        if (!manager->active_faults[i].is_active)
        {
            return i;
        }
    }

    return -1;
}

static void recalculate_highest_fault_level(FaultManager *manager)
{
    uint8_t i;
    FaultLevel highest = FAULT_LEVEL_NO_FAULT;
    uint8_t count = 0;

    if (manager == 0)
    {
        return;
    }

    for (i = 0; i < FAULT_MANAGER_MAX_ACTIVE_FAULTS; ++i)
    {
        if (manager->active_faults[i].is_active)
        {
            ++count;

            if (manager->active_faults[i].level > highest)
            {
                highest = manager->active_faults[i].level;
            }
        }
    }

    manager->active_fault_count = count;
    manager->highest_fault_level = highest;
}

void FaultManager_Init(FaultManager *manager)
{
    uint8_t i;

    if (manager == 0)
    {
        return;
    }

    for (i = 0; i < FAULT_MANAGER_MAX_ACTIVE_FAULTS; ++i)
    {
        manager->active_faults[i].code = FAULT_CODE_NONE;
        manager->active_faults[i].level = FAULT_LEVEL_NO_FAULT;
        manager->active_faults[i].is_active = 0;
        manager->active_faults[i].occurrence_count = 0;
    }

    manager->active_fault_count = 0;
    manager->highest_fault_level = FAULT_LEVEL_NO_FAULT;
}

int FaultManager_ReportFault(FaultManager *manager, FaultCode code, FaultLevel level)
{
    int index;

    if (manager == 0 || code == FAULT_CODE_NONE)
    {
        return 0;
    }

    index = find_fault_index(manager, code);

    if (index >= 0)
    {
        manager->active_faults[index].level = level;
        manager->active_faults[index].occurrence_count++;
        recalculate_highest_fault_level(manager);
        return 1;
    }

    index = find_free_slot(manager);

    if (index < 0)
    {
        return 0;
    }

    manager->active_faults[index].code = code;
    manager->active_faults[index].level = level;
    manager->active_faults[index].is_active = 1;
    manager->active_faults[index].occurrence_count = 1;

    recalculate_highest_fault_level(manager);

    return 1;
}

int FaultManager_ClearFault(FaultManager *manager, FaultCode code)
{
    int index;

    if (manager == 0 || code == FAULT_CODE_NONE)
    {
        return 0;
    }

    index = find_fault_index(manager, code);

    if (index < 0)
    {
        return 0;
    }

    manager->active_faults[index].code = FAULT_CODE_NONE;
    manager->active_faults[index].level = FAULT_LEVEL_NO_FAULT;
    manager->active_faults[index].is_active = 0;

    recalculate_highest_fault_level(manager);

    return 1;
}

uint8_t FaultManager_IsFaultActive(const FaultManager *manager, FaultCode code)
{
    return find_fault_index(manager, code) >= 0 ? 1 : 0;
}

FaultLevel FaultManager_GetHighestFaultLevel(const FaultManager *manager)
{
    if (manager == 0)
    {
        return FAULT_LEVEL_CATASTROPHIC;
    }

    return manager->highest_fault_level;
}

uint8_t FaultManager_GetActiveFaultCount(const FaultManager *manager)
{
    if (manager == 0)
    {
        return 0;
    }

    return manager->active_fault_count;
}

SystemEvent FaultManager_MapFaultLevelToSystemEvent(FaultLevel level)
{
    switch (level)
    {
    case FAULT_LEVEL_NO_FAULT:
        return SYSTEM_EVENT_SELF_TEST_PASSED;

    case FAULT_LEVEL_MINOR:
        return SYSTEM_EVENT_MINOR_FAULT_DETECTED;

    case FAULT_LEVEL_MAJOR:
        return SYSTEM_EVENT_MAJOR_FAULT_DETECTED;

    case FAULT_LEVEL_CRITICAL:
    case FAULT_LEVEL_CATASTROPHIC:
        return SYSTEM_EVENT_CRITICAL_FAULT_DETECTED;

    default:
        return SYSTEM_EVENT_CRITICAL_FAULT_DETECTED;
    }
}

const char *FaultCode_ToString(FaultCode code)
{
    switch (code)
    {
    case FAULT_CODE_NONE:
        return "NONE";

    case FAULT_CODE_SENSOR_TEMPERATURE_HIGH:
        return "SENSOR_TEMPERATURE_HIGH";

    case FAULT_CODE_SENSOR_VOLTAGE_LOW:
        return "SENSOR_VOLTAGE_LOW";

    case FAULT_CODE_SENSOR_VOLTAGE_HIGH:
        return "SENSOR_VOLTAGE_HIGH";

    case FAULT_CODE_ACTUATOR_POSITION_INVALID:
        return "ACTUATOR_POSITION_INVALID";

    case FAULT_CODE_COMMUNICATION_TIMEOUT:
        return "COMMUNICATION_TIMEOUT";

    case FAULT_CODE_WATCHDOG_EXPIRED:
        return "WATCHDOG_EXPIRED";

    case FAULT_CODE_SELF_TEST_FAILED:
        return "SELF_TEST_FAILED";

    case FAULT_CODE_UNKNOWN:
        return "UNKNOWN";

    default:
        return "INVALID_FAULT_CODE";
    }
}