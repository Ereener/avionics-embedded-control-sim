#include "watchdog.h"

static void update_status(Watchdog *watchdog)
{
    if (watchdog == 0)
    {
        return;
    }

    if (watchdog->elapsed_ticks >= watchdog->timeout_ticks)
    {
        watchdog->status = WATCHDOG_STATUS_EXPIRED;
    }
    else if (watchdog->elapsed_ticks >= watchdog->warning_threshold_ticks)
    {
        watchdog->status = WATCHDOG_STATUS_WARNING;
    }
    else
    {
        watchdog->status = WATCHDOG_STATUS_OK;
    }
}

void Watchdog_Init(Watchdog *watchdog, uint32_t timeout_ticks, uint32_t warning_threshold_ticks)
{
    if (watchdog == 0)
    {
        return;
    }

    watchdog->timeout_ticks = timeout_ticks;
    watchdog->warning_threshold_ticks = warning_threshold_ticks;
    watchdog->elapsed_ticks = 0;
    watchdog->status = WATCHDOG_STATUS_OK;

    if (watchdog->warning_threshold_ticks >= watchdog->timeout_ticks)
    {
        watchdog->warning_threshold_ticks = watchdog->timeout_ticks / 2;
    }
}

void Watchdog_Tick(Watchdog *watchdog)
{
    if (watchdog == 0)
    {
        return;
    }

    if (watchdog->elapsed_ticks < watchdog->timeout_ticks)
    {
        watchdog->elapsed_ticks++;
    }

    update_status(watchdog);
}

void Watchdog_Feed(Watchdog *watchdog)
{
    if (watchdog == 0)
    {
        return;
    }

    watchdog->elapsed_ticks = 0;
    watchdog->status = WATCHDOG_STATUS_OK;
}

WatchdogStatus Watchdog_GetStatus(const Watchdog *watchdog)
{
    if (watchdog == 0)
    {
        return WATCHDOG_STATUS_EXPIRED;
    }

    return watchdog->status;
}

uint8_t Watchdog_IsExpired(const Watchdog *watchdog)
{
    if (watchdog == 0)
    {
        return 1;
    }

    return watchdog->status == WATCHDOG_STATUS_EXPIRED ? 1 : 0;
}

const char *WatchdogStatus_ToString(WatchdogStatus status)
{
    switch (status)
    {
    case WATCHDOG_STATUS_OK:
        return "OK";

    case WATCHDOG_STATUS_WARNING:
        return "WARNING";

    case WATCHDOG_STATUS_EXPIRED:
        return "EXPIRED";

    default:
        return "UNKNOWN_WATCHDOG_STATUS";
    }
}