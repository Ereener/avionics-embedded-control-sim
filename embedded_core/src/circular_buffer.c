#include "circular_buffer.h"

void CircularBuffer_Init(CircularBuffer *buffer)
{
    uint16_t i;

    if (buffer == 0)
    {
        return;
    }

    for (i = 0; i < CIRCULAR_BUFFER_CAPACITY; ++i)
    {
        buffer->data[i] = 0;
    }

    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
}

CircularBufferStatus CircularBuffer_Push(CircularBuffer *buffer, uint8_t value)
{
    if (buffer == 0)
    {
        return CIRCULAR_BUFFER_NULL;
    }

    if (buffer->count >= CIRCULAR_BUFFER_CAPACITY)
    {
        return CIRCULAR_BUFFER_FULL;
    }

    buffer->data[buffer->head] = value;
    buffer->head = (uint16_t)((buffer->head + 1U) % CIRCULAR_BUFFER_CAPACITY);
    buffer->count++;

    return CIRCULAR_BUFFER_OK;
}

CircularBufferStatus CircularBuffer_Pop(CircularBuffer *buffer, uint8_t *value)
{
    if (buffer == 0 || value == 0)
    {
        return CIRCULAR_BUFFER_NULL;
    }

    if (buffer->count == 0)
    {
        return CIRCULAR_BUFFER_EMPTY;
    }

    *value = buffer->data[buffer->tail];
    buffer->tail = (uint16_t)((buffer->tail + 1U) % CIRCULAR_BUFFER_CAPACITY);
    buffer->count--;

    return CIRCULAR_BUFFER_OK;
}

CircularBufferStatus CircularBuffer_Peek(const CircularBuffer *buffer, uint8_t *value)
{
    if (buffer == 0 || value == 0)
    {
        return CIRCULAR_BUFFER_NULL;
    }

    if (buffer->count == 0)
    {
        return CIRCULAR_BUFFER_EMPTY;
    }

    *value = buffer->data[buffer->tail];

    return CIRCULAR_BUFFER_OK;
}

uint8_t CircularBuffer_IsFull(const CircularBuffer *buffer)
{
    if (buffer == 0)
    {
        return 0;
    }

    return buffer->count >= CIRCULAR_BUFFER_CAPACITY ? 1U : 0U;
}

uint8_t CircularBuffer_IsEmpty(const CircularBuffer *buffer)
{
    if (buffer == 0)
    {
        return 1U;
    }

    return buffer->count == 0 ? 1U : 0U;
}

uint16_t CircularBuffer_GetCount(const CircularBuffer *buffer)
{
    if (buffer == 0)
    {
        return 0;
    }

    return buffer->count;
}

uint16_t CircularBuffer_GetCapacity(void)
{
    return CIRCULAR_BUFFER_CAPACITY;
}

void CircularBuffer_Clear(CircularBuffer *buffer)
{
    if (buffer == 0)
    {
        return;
    }

    buffer->head = 0;
    buffer->tail = 0;
    buffer->count = 0;
}

const char *CircularBufferStatus_ToString(CircularBufferStatus status)
{
    switch (status)
    {
    case CIRCULAR_BUFFER_OK:
        return "OK";

    case CIRCULAR_BUFFER_FULL:
        return "FULL";

    case CIRCULAR_BUFFER_EMPTY:
        return "EMPTY";

    case CIRCULAR_BUFFER_NULL:
        return "NULL";

    default:
        return "UNKNOWN_CIRCULAR_BUFFER_STATUS";
    }
}