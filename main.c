#include <stdio.h>
#include "system_state.h"
#include "sensor_monitor.h"

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

int main(void)
{
    SystemStateMachine machine;
    SensorMonitor sensor_monitor;

    SystemState_Init(&machine);
    SensorMonitor_Init(&sensor_monitor);

    SystemState_HandleEvent(&machine, SYSTEM_EVENT_POWER_ON);
    SystemState_HandleEvent(&machine, SYSTEM_EVENT_SELF_TEST_PASSED);

    printf("=== System State ===\n");
    print_system_status(&machine);

    printf("\n=== Nominal Sensor Values ===\n");
    SensorMonitor_UpdateTemperature(&sensor_monitor, 2000);
    SensorMonitor_UpdateVoltage(&sensor_monitor, 3000);
    SensorMonitor_UpdateActuatorPosition(&sensor_monitor, 2048);
    print_sensor_status(&sensor_monitor);

    printf("\n=== High Temperature Scenario ===\n");
    SensorMonitor_UpdateTemperature(&sensor_monitor, 3800);
    print_sensor_status(&sensor_monitor);

    printf("\n=== Low Voltage Scenario ===\n");
    SensorMonitor_UpdateVoltage(&sensor_monitor, 1500);
    print_sensor_status(&sensor_monitor);

    return 0;
}