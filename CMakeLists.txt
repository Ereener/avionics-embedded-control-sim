cmake_minimum_required(VERSION 3.16)

project(AvionicsEmbeddedControlSim C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    embedded_core/include
)

add_executable(avionics_sim
    main.c
    embedded_core/src/system_state.c
    embedded_core/src/sensor_monitor.c
    embedded_core/src/fault_manager.c
    embedded_core/src/watchdog.c
    embedded_core/src/circular_buffer.c
    embedded_core/src/uart_comm.c
    embedded_core/src/can_comm.c
)
