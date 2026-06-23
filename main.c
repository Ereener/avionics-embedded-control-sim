#include <stdio.h>
#include "system_state.h"
#include "sensor_monitor.h"
#include "fault_manager.h"
#include "watchdog.h"
#include "circular_buffer.h"
#include "uart_comm.h"
#include "can_comm.h"

static void print_system_status(const SystemStateMachine *machine)
{
    printf("State: %s | Fault: %s\n",
           SystemState_ToString(SystemState_GetCurrentState(machine)),
           FaultLevel_ToString(SystemState_GetCurrentFault(machine)));
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

static void demo_circular_buffer(void)
{
    CircularBuffer rx_buffer;
    CircularBufferStatus status;
    uint8_t value;
    uint8_t received_bytes[] = {0xAA, 0x10, 0x20, 0x30, 0x55};
    uint8_t i;

    CircularBuffer_Init(&rx_buffer);

    printf("\n=== Circular Buffer UART/RS485 RX Simulation ===\n");

    for (i = 0; i < sizeof(received_bytes); ++i)
    {
        status = CircularBuffer_Push(&rx_buffer, received_bytes[i]);

        printf("RX byte: 0x%02X | Push Status: %s | Count: %u\n",
               received_bytes[i],
               CircularBufferStatus_ToString(status),
               CircularBuffer_GetCount(&rx_buffer));
    }

    printf("\nProcessing received bytes FIFO order:\n");

    while (!CircularBuffer_IsEmpty(&rx_buffer))
    {
        status = CircularBuffer_Pop(&rx_buffer, &value);

        printf("Processed byte: 0x%02X | Pop Status: %s | Remaining: %u\n",
               value,
               CircularBufferStatus_ToString(status),
               CircularBuffer_GetCount(&rx_buffer));
    }
}

static void demo_uart_comm(void)
{
    CircularBuffer rx_buffer;
    UartCommParser parser;
    UartCommFrame frame;
    UartCommStatus status;
    uint8_t received_bytes[] = {0x00, 0xAA, 0x01, 0x02, 0xA5, 0x5A, 0xFC, 0x55};
    uint8_t index;

    CircularBuffer_Init(&rx_buffer);
    UartComm_Init(&parser);

    for (index = 0U; index < sizeof(received_bytes); ++index)
    {
        (void)CircularBuffer_Push(&rx_buffer, received_bytes[index]);
    }

    status = UartComm_ProcessRx(&parser, &rx_buffer, &frame);

    printf("\n=== UART Communication Frame Parser ===\n");
    printf("Parser Status: %s\n", UartCommStatus_ToString(status));

    if (status == UART_COMM_FRAME_READY)
    {
        printf("Message ID: 0x%02X | Length: %u | Payload:", frame.message_id, frame.length);
        for (index = 0U; index < frame.length; ++index)
        {
            printf(" 0x%02X", frame.payload[index]);
        }
        printf("\n");
    }
}

static void demo_can_comm(void)
{
    CircularBuffer rx_buffer;
    CanCommParser parser;
    CanCommFrame frame;
    CanCommStatus status;
    uint8_t received_bytes[] = {0xC5, 0x01, 0x23, 0x02, 0x10, 0x20, 0x5C};
    uint8_t index;

    CircularBuffer_Init(&rx_buffer);
    CanComm_Init(&parser);

    for (index = 0U; index < sizeof(received_bytes); ++index)
    {
        (void)CircularBuffer_Push(&rx_buffer, received_bytes[index]);
    }

    status = CanComm_ProcessRx(&parser, &rx_buffer, &frame);

    printf("\n=== CAN Communication Frame Parser ===\n");
    printf("Parser Status: %s\n", CanCommStatus_ToString(status));

    if (status == CAN_COMM_FRAME_READY)
    {
        printf("CAN ID: 0x%03X | DLC: %u | Data:", frame.identifier, frame.dlc);
        for (index = 0U; index < frame.dlc; ++index)
        {
            printf(" 0x%02X", frame.data[index]);
        }
        printf("\n");
    }
}

int main(void)
{
    SystemStateMachine machine;
    FaultManager fault_manager;
    Watchdog watchdog;
    uint32_t tick;

    SystemState_Init(&machine);
    FaultManager_Init(&fault_manager);
    Watchdog_Init(&watchdog, 5, 3);

    SystemState_HandleEvent(&machine, SYSTEM_EVENT_POWER_ON);
    SystemState_HandleEvent(&machine, SYSTEM_EVENT_SELF_TEST_PASSED);

    printf("=== Initial System State ===\n");
    print_system_status(&machine);

    demo_circular_buffer();
    demo_uart_comm();
    demo_can_comm();

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

    return 0;
}
