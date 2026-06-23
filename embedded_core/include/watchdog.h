#ifndef WATCHDOG_H
#define WATCHDOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef enum
{
    WATCHDOG_STATUS_OK = 0,
    WATCHDOG_STATUS_WARNING,
    WATCHDOG_STATUS_EXPIRED
} WatchdogStatus;

typedef struct
{
    uint32_t timeout_ticks;
    uint32_t elapsed_ticks;
    uint32_t warning_threshold_ticks;
    WatchdogStatus status;
} Watchdog;

void Watchdog_Init(Watchdog *watchdog, uint32_t timeout_ticks, uint32_t warning_threshold_ticks);

void Watchdog_Tick(Watchdog *watchdog);

void Watchdog_Feed(Watchdog *watchdog);

WatchdogStatus Watchdog_GetStatus(const Watchdog *watchdog);

uint8_t Watchdog_IsExpired(const Watchdog *watchdog);

const char *WatchdogStatus_ToString(WatchdogStatus status);

#ifdef __cplusplus
}
#endif

#endif