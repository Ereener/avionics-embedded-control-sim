#include "can_comm.h"

static void CanComm_ResetParser(CanCommParser *parser)
{
    parser->state = CAN_COMM_WAITING_FOR_START;
    parser->data_index = 0U;
    parser->calculated_checksum = 0U;
}

void CanComm_Init(CanCommParser *parser)
{
    uint8_t index;

    if (parser == 0)
    {
        return;
    }

    parser->working_frame.identifier = 0U;
    parser->working_frame.dlc = 0U;
    parser->working_frame.checksum = 0U;

    for (index = 0U; index < CAN_COMM_MAX_DLC; ++index)
    {
        parser->working_frame.data[index] = 0U;
    }

    CanComm_ResetParser(parser);
}

uint8_t CanComm_CalculateChecksum(const CanCommFrame *frame)
{
    uint8_t checksum;
    uint8_t index;

    if (frame == 0 || frame->identifier > CAN_COMM_STANDARD_ID_MAX || frame->dlc > CAN_COMM_MAX_DLC)
    {
        return 0U;
    }

    checksum = (uint8_t)(((uint8_t)(frame->identifier >> 8U)) ^
                         ((uint8_t)frame->identifier) ^ frame->dlc);

    for (index = 0U; index < frame->dlc; ++index)
    {
        checksum = (uint8_t)(checksum ^ frame->data[index]);
    }

    return checksum;
}

uint8_t CanComm_IsFrameValid(const CanCommFrame *frame)
{
    if (frame == 0 || frame->identifier > CAN_COMM_STANDARD_ID_MAX || frame->dlc > CAN_COMM_MAX_DLC)
    {
        return 0U;
    }

    return frame->checksum == CanComm_CalculateChecksum(frame) ? 1U : 0U;
}

CanCommStatus CanComm_ProcessRx(CanCommParser *parser,
                                CircularBuffer *rx_buffer,
                                CanCommFrame *frame)
{
    uint8_t received_byte;

    if (parser == 0 || rx_buffer == 0 || frame == 0)
    {
        return CAN_COMM_NULL;
    }

    while (CircularBuffer_Pop(rx_buffer, &received_byte) == CIRCULAR_BUFFER_OK)
    {
        switch (parser->state)
        {
        case CAN_COMM_WAITING_FOR_START:
            if (received_byte == CAN_COMM_START_BYTE)
            {
                parser->state = CAN_COMM_READING_ID_HIGH;
            }
            break;

        case CAN_COMM_READING_ID_HIGH:
            if (received_byte > 0x07U)
            {
                CanComm_ResetParser(parser);
                return CAN_COMM_INVALID_IDENTIFIER;
            }
            parser->working_frame.identifier = (uint16_t)(received_byte << 8U);
            parser->calculated_checksum = received_byte;
            parser->state = CAN_COMM_READING_ID_LOW;
            break;

        case CAN_COMM_READING_ID_LOW:
            parser->working_frame.identifier = (uint16_t)(parser->working_frame.identifier | received_byte);
            parser->calculated_checksum = (uint8_t)(parser->calculated_checksum ^ received_byte);
            parser->state = CAN_COMM_READING_DLC;
            break;

        case CAN_COMM_READING_DLC:
            if (received_byte > CAN_COMM_MAX_DLC)
            {
                CanComm_ResetParser(parser);
                return CAN_COMM_INVALID_DLC;
            }
            parser->working_frame.dlc = received_byte;
            parser->data_index = 0U;
            parser->calculated_checksum = (uint8_t)(parser->calculated_checksum ^ received_byte);
            parser->state = received_byte == 0U ? CAN_COMM_READING_CHECKSUM : CAN_COMM_READING_DATA;
            break;

        case CAN_COMM_READING_DATA:
            parser->working_frame.data[parser->data_index] = received_byte;
            parser->data_index++;
            parser->calculated_checksum = (uint8_t)(parser->calculated_checksum ^ received_byte);
            if (parser->data_index >= parser->working_frame.dlc)
            {
                parser->state = CAN_COMM_READING_CHECKSUM;
            }
            break;

        case CAN_COMM_READING_CHECKSUM:
            parser->working_frame.checksum = received_byte;
            parser->state = CAN_COMM_READING_END;
            break;

        case CAN_COMM_READING_END:
            if (received_byte != CAN_COMM_END_BYTE)
            {
                parser->state = received_byte == CAN_COMM_START_BYTE
                                    ? CAN_COMM_READING_ID_HIGH
                                    : CAN_COMM_WAITING_FOR_START;
                return CAN_COMM_INVALID_END_BYTE;
            }
            if (parser->working_frame.checksum != parser->calculated_checksum)
            {
                CanComm_ResetParser(parser);
                return CAN_COMM_CHECKSUM_ERROR;
            }
            *frame = parser->working_frame;
            CanComm_ResetParser(parser);
            return CAN_COMM_FRAME_READY;

        default:
            CanComm_ResetParser(parser);
            break;
        }
    }

    return CAN_COMM_NO_DATA;
}

const char *CanCommStatus_ToString(CanCommStatus status)
{
    switch (status)
    {
    case CAN_COMM_FRAME_READY: return "FRAME_READY";
    case CAN_COMM_NO_DATA: return "NO_DATA";
    case CAN_COMM_INVALID_IDENTIFIER: return "INVALID_IDENTIFIER";
    case CAN_COMM_INVALID_DLC: return "INVALID_DLC";
    case CAN_COMM_CHECKSUM_ERROR: return "CHECKSUM_ERROR";
    case CAN_COMM_INVALID_END_BYTE: return "INVALID_END_BYTE";
    case CAN_COMM_NULL: return "NULL";
    default: return "UNKNOWN_CAN_COMM_STATUS";
    }
}
