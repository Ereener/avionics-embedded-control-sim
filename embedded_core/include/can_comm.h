#ifndef CAN_COMM_H
#define CAN_COMM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "circular_buffer.h"

/*
 * Host-side CAN simulation frame:
 * START | ID[10:8] | ID[7:0] | DLC | DATA[0..DLC-1] | CHECKSUM | END
 *  0xC5 |   0x00-07 |   byte   | 0-8 |     bytes      |   XOR    | 0x5C
 *
 * This is a software transport envelope, not the electrical CAN bit stream.
 * A real CAN controller handles SOF, arbitration, CRC, ACK and bit stuffing.
 */
#define CAN_COMM_START_BYTE 0xC5U
#define CAN_COMM_END_BYTE 0x5CU
#define CAN_COMM_STANDARD_ID_MAX 0x07FFU
#define CAN_COMM_MAX_DLC 8U

typedef struct
{
    uint16_t identifier;
    uint8_t dlc;
    uint8_t data[CAN_COMM_MAX_DLC];
    uint8_t checksum;
} CanCommFrame;

typedef enum
{
    CAN_COMM_WAITING_FOR_START = 0,
    CAN_COMM_READING_ID_HIGH,
    CAN_COMM_READING_ID_LOW,
    CAN_COMM_READING_DLC,
    CAN_COMM_READING_DATA,
    CAN_COMM_READING_CHECKSUM,
    CAN_COMM_READING_END
} CanCommParserState;

typedef enum
{
    CAN_COMM_FRAME_READY = 0,
    CAN_COMM_NO_DATA,
    CAN_COMM_INVALID_IDENTIFIER,
    CAN_COMM_INVALID_DLC,
    CAN_COMM_CHECKSUM_ERROR,
    CAN_COMM_INVALID_END_BYTE,
    CAN_COMM_NULL
} CanCommStatus;

typedef struct
{
    CanCommParserState state;
    CanCommFrame working_frame;
    uint8_t data_index;
    uint8_t calculated_checksum;
} CanCommParser;

void CanComm_Init(CanCommParser *parser);

CanCommStatus CanComm_ProcessRx(CanCommParser *parser,
                                CircularBuffer *rx_buffer,
                                CanCommFrame *frame);

uint8_t CanComm_CalculateChecksum(const CanCommFrame *frame);

uint8_t CanComm_IsFrameValid(const CanCommFrame *frame);

const char *CanCommStatus_ToString(CanCommStatus status);

#ifdef __cplusplus
}
#endif

#endif
