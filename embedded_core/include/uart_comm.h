#ifndef UART_COMM_H
#define UART_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "circular_buffer.h"

#define UART_COMM_START_BYTE 0xAAU
#define UART_COMM_END_BYTE 0x55U
#define UART_COMM_MAX_PAYLOAD_LENGTH 48U

typedef struct
{
    uint8_t message_id;
    uint8_t length;
    uint8_t payload[UART_COMM_MAX_PAYLOAD_LENGTH];
} UartCommFrame;

typedef enum
{
    UART_COMM_WAITING_FOR_START = 0,
    UART_COMM_READING_MESSAGE_ID,
    UART_COMM_READING_LENGTH,
    UART_COMM_READING_PAYLOAD,
    UART_COMM_READING_CHECKSUM,
    UART_COMM_READING_END
} UartCommParserState;

typedef enum
{
    UART_COMM_FRAME_READY = 0,
    UART_COMM_NO_DATA,
    UART_COMM_INVALID_LENGTH,
    UART_COMM_CHECKSUM_ERROR,
    UART_COMM_INVALID_END_BYTE,
    UART_COMM_NULL
} UartCommStatus;

typedef struct
{
    UartCommParserState state;
    UartCommFrame working_frame;
    uint8_t payload_index;
    uint8_t calculated_checksum;
    uint8_t received_checksum;
} UartCommParser;

void UartComm_Init(UartCommParser *parser);

UartCommStatus UartComm_ProcessRx(UartCommParser *parser,
                                  CircularBuffer *rx_buffer,
                                  UartCommFrame *frame);

uint8_t UartComm_CalculateChecksum(const UartCommFrame *frame);

const char *UartCommStatus_ToString(UartCommStatus status);

#ifdef __cplusplus
}
#endif

#endif
