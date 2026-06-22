#include <stdio.h>
#include "system_state.h"

static void print_system_status(const SystemStateMachine *machine)
{
    printf("State: %s | Fault: %s\n",
           SystemState_ToString(SystemState_GetCurrentState(machine)),
           FaultLevel_ToString(SystemState_GetCurrentFault(machine)));
}

int main(void)
{
    SystemStateMachine machine;

    SystemState_Init(&machine);
    print_system_status(&machine);

    SystemState_HandleEvent(&machine, SYSTEM_EVENT_POWER_ON);
    print_system_status(&machine);

    SystemState_HandleEvent(&machine, SYSTEM_EVENT_SELF_TEST_PASSED);
    print_system_status(&machine);

    SystemState_HandleEvent(&machine, SYSTEM_EVENT_MINOR_FAULT_DETECTED);
    print_system_status(&machine);

    SystemState_HandleEvent(&machine, SYSTEM_EVENT_CRITICAL_FAULT_DETECTED);
    print_system_status(&machine);

    SystemState_HandleEvent(&machine, SYSTEM_EVENT_MAINTENANCE_REQUESTED);
    print_system_status(&machine);

    SystemState_HandleEvent(&machine, SYSTEM_EVENT_RESET_REQUESTED);
    print_system_status(&machine);

    return 0;
}