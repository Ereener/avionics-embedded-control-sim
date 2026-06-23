#include "uart_comm.h"

static void UartComm_ResetParser(UartCommParser *parser)
{
    parser->state = UART_COMM_WAITING_FOR_START;
    parser->payload_index = 0U;
    parser->calculated_checksum = 0U;
    parser->received_checksum = 0U;
}

void UartComm_Init(UartCommParser *parser)
{
    uint8_t index;

    if (parser == 0)
    {
        return;
    }

    parser->working_frame.message_id = 0U;
    parser->working_frame.length = 0U;
    for (index = 0U; index < UART_COMM_MAX_PAYLOAD_LENGTH; ++index)
    {
        parser->working_frame.payload[index] = 0U;
    }

    UartComm_ResetParser(parser);
}

uint8_t UartComm_CalculateChecksum(const UartCommFrame *frame)
{
    uint8_t checksum;
    uint8_t index;

    if (frame == 0)
    {
        return 0U;
    }

    checksum = (uint8_t)(frame->message_id ^ frame->length);
    for (index = 0U; index < frame->length; ++index)
    {
        checksum = (uint8_t)(checksum ^ frame->payload[index]);
    }

    return checksum;
}

UartCommStatus UartComm_ProcessRx(UartCommParser *parser,
                                  CircularBuffer *rx_buffer,
                                  UartCommFrame *frame)
{
    uint8_t received_byte;

    if (parser == 0 || rx_buffer == 0 || frame == 0)
    {
        return UART_COMM_NULL;
    }

    while (CircularBuffer_Pop(rx_buffer, &received_byte) == CIRCULAR_BUFFER_OK)
    {
        switch (parser->state)
        {
        case UART_COMM_WAITING_FOR_START:
            if (received_byte == UART_COMM_START_BYTE)
            {
                parser->state = UART_COMM_READING_MESSAGE_ID;
            }
            break;

        case UART_COMM_READING_MESSAGE_ID:
            parser->working_frame.message_id = received_byte;
            parser->calculated_checksum = received_byte;
            parser->state = UART_COMM_READING_LENGTH;
            break;

        case UART_COMM_READING_LENGTH:
            if (received_byte > UART_COMM_MAX_PAYLOAD_LENGTH)
            {
                UartComm_ResetParser(parser);
                return UART_COMM_INVALID_LENGTH;
            }
            parser->working_frame.length = received_byte;
            parser->payload_index = 0U;
            parser->calculated_checksum = (uint8_t)(parser->calculated_checksum ^ received_byte);
            parser->state = received_byte == 0U ? UART_COMM_READING_CHECKSUM : UART_COMM_READING_PAYLOAD;
            break;

        case UART_COMM_READING_PAYLOAD:
            parser->working_frame.payload[parser->payload_index] = received_byte;
            parser->payload_index++;
            parser->calculated_checksum = (uint8_t)(parser->calculated_checksum ^ received_byte);
            if (parser->payload_index >= parser->working_frame.length)
            {
                parser->state = UART_COMM_READING_CHECKSUM;
            }
            break;

        case UART_COMM_READING_CHECKSUM:
            parser->received_checksum = received_byte;
            parser->state = UART_COMM_READING_END;
            break;

        case UART_COMM_READING_END:
            if (received_byte != UART_COMM_END_BYTE)
            {
                parser->state = received_byte == UART_COMM_START_BYTE
                                    ? UART_COMM_READING_MESSAGE_ID
                                    : UART_COMM_WAITING_FOR_START;
                return UART_COMM_INVALID_END_BYTE;
            }
            if (parser->received_checksum != parser->calculated_checksum)
            {
                UartComm_ResetParser(parser);
                return UART_COMM_CHECKSUM_ERROR;
            }
            *frame = parser->working_frame;
            UartComm_ResetParser(parser);
            return UART_COMM_FRAME_READY;

        default:
            UartComm_ResetParser(parser);
            break;
        }
    }

    return UART_COMM_NO_DATA;
}

const char *UartCommStatus_ToString(UartCommStatus status)
{
    switch (status)
    {
    case UART_COMM_FRAME_READY: return "FRAME_READY";
    case UART_COMM_NO_DATA: return "NO_DATA";
    case UART_COMM_INVALID_LENGTH: return "INVALID_LENGTH";
    case UART_COMM_CHECKSUM_ERROR: return "CHECKSUM_ERROR";
    case UART_COMM_INVALID_END_BYTE: return "INVALID_END_BYTE";
    case UART_COMM_NULL: return "NULL";
    default: return "UNKNOWN_UART_COMM_STATUS";
    }
}
