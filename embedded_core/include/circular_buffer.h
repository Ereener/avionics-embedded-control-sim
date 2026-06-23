#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define CIRCULAR_BUFFER_CAPACITY 64

typedef enum
{
    CIRCULAR_BUFFER_OK = 0,
    CIRCULAR_BUFFER_FULL,
    CIRCULAR_BUFFER_EMPTY,
    CIRCULAR_BUFFER_NULL
} CircularBufferStatus;

typedef struct
{
    uint8_t data[CIRCULAR_BUFFER_CAPACITY];
    uint16_t head;
    uint16_t tail;
    uint16_t count;
} CircularBuffer;

void CircularBuffer_Init(CircularBuffer *buffer);

CircularBufferStatus CircularBuffer_Push(CircularBuffer *buffer, uint8_t value);

CircularBufferStatus CircularBuffer_Pop(CircularBuffer *buffer, uint8_t *value);

CircularBufferStatus CircularBuffer_Peek(const CircularBuffer *buffer, uint8_t *value);

uint8_t CircularBuffer_IsFull(const CircularBuffer *buffer);

uint8_t CircularBuffer_IsEmpty(const CircularBuffer *buffer);

uint16_t CircularBuffer_GetCount(const CircularBuffer *buffer);

uint16_t CircularBuffer_GetCapacity(void);

void CircularBuffer_Clear(CircularBuffer *buffer);

const char *CircularBufferStatus_ToString(CircularBufferStatus status);

#ifdef __cplusplus
}
#endif

#endif