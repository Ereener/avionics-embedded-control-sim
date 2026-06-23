#include <stdio.h>
#include "system_state.h"
#include "sensor_monitor.h"
#include "fault_manager.h"
#include "watchdog.h"

static void print_system_status(const SystemStateMachine *machine)
{
    printf("State: %s | Fault: %s\n",
           SystemState_ToString(SystemState_GetCurrentState(machine)),
           FaultLevel_ToString(SystemState_GetCurrentFault(machine)));
}

static void print_sensor_status(const SensorMonitor *monitor)
{
    printf("Temperature: %.2f C | Status: %s\n",
           monitor->temperature_celsius.engineering_value,
           SensorStatus_ToString(monitor->temperature_celsius.status));

    printf("Voltage: %.2f V | Status: %s\n",
           monitor->voltage_volts.engineering_value,
           SensorStatus_ToString(monitor->voltage_volts.status));

    printf("Actuator Position: %.2f %% | Status: %s\n",
           monitor->actuator_position_percent.engineering_value,
           SensorStatus_ToString(monitor->actuator_position_percent.status));

    printf("Sensor Overall Fault: %s\n",
           FaultLevel_ToString(SensorMonitor_GetOverallFaultLevel(monitor)));
}

static void print_fault_manager_status(const FaultManager *manager)
{
    printf("Active Fault Count: %u\n", FaultManager_GetActiveFaultCount(manager));
    printf("Highest Fault Level: %s\n",
           FaultLevel_ToString(FaultManager_GetHighestFaultLevel(manager)));
}

static void print_watchdog_status(const Watchdog *watchdog)
{
    printf("Watchdog elapsed ticks: %lu | Status: %s\n",
           (unsigned long)watchdog->elapsed_ticks,
           WatchdogStatus_ToString(Watchdog_GetStatus(watchdog)));
}

static void process_faults(SystemStateMachine *machine, FaultManager *fault_manager)
{
    SystemEvent fault_event;

    fault_event = FaultManager_MapFaultLevelToSystemEvent(
        FaultManager_GetHighestFaultLevel(fault_manager));

    SystemState_HandleEvent(machine, fault_event);
}

int main(void)
{
    SystemStateMachine machine;
    SensorMonitor sensor_monitor;
    FaultManager fault_manager;
    Watchdog watchdog;
    uint32_t tick;

    SystemState_Init(&machine);
    SensorMonitor_Init(&sensor_monitor);
    FaultManager_Init(&fault_manager);
    Watchdog_Init(&watchdog, 5, 3);

    SystemState_HandleEvent(&machine, SYSTEM_EVENT_POWER_ON);
    SystemState_HandleEvent(&machine, SYSTEM_EVENT_SELF_TEST_PASSED);

    printf("=== Initial System State ===\n");
    print_system_status(&machine);

    printf("\n=== Watchdog Normal Operation ===\n");

    for (tick = 0; tick < 3; ++tick)
    {
        Watchdog_Tick(&watchdog);
        print_watchdog_status(&watchdog);
    }

    printf("Feeding watchdog...\n");
    Watchdog_Feed(&watchdog);
    print_watchdog_status(&watchdog);

    printf("\n=== Watchdog Timeout Scenario ===\n");

    for (tick = 0; tick < 6; ++tick)
    {
        Watchdog_Tick(&watchdog);
        print_watchdog_status(&watchdog);
    }

    if (Watchdog_IsExpired(&watchdog))
    {
        FaultManager_ReportFault(
            &fault_manager,
            FAULT_CODE_WATCHDOG_EXPIRED,
            FAULT_LEVEL_CRITICAL);
    }

    print_fault_manager_status(&fault_manager);
    process_faults(&machine, &fault_manager);

    printf("\n=== System State After Watchdog Expiration ===\n");
    print_system_status(&machine);

    printf("\n=== Sensor Fault Scenario ===\n");

    SensorMonitor_UpdateTemperature(&sensor_monitor, 3800);

    if (SensorMonitor_GetOverallFaultLevel(&sensor_monitor) >= FAULT_LEVEL_MAJOR)
    {
        FaultManager_ReportFault(
            &fault_manager,
            FAULT_CODE_SENSOR_TEMPERATURE_HIGH,
            SensorMonitor_GetOverallFaultLevel(&sensor_monitor));
    }

    print_sensor_status(&sensor_monitor);
    print_fault_manager_status(&fault_manager);
    process_faults(&machine, &fault_manager);

    printf("\n=== Final System State ===\n");
    print_system_status(&machine);

    return 0;
}