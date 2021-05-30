#ifndef _CANIOT_DEF_H
#define _CANIOT_DEF_H

#include <stdint.h>

#include <mcp2515_can_dfs.h>

/*___________________________________________________________________________*/

#define FRAME_COMMAND           0b00
#define FRAME_TELEMETRY         0b01
#define FRAME_WRITE_ATTRIBUTE   0b10
#define FRAME_READ_ATTRIBUTE    0b11

#define FRAME_QUERY             0b1
#define FRAME_RESPONSE          0b0

#define CONTROLLER_MAIN         0b00
#define CONTROLLER_1            0b01
#define CONTROLLER_2            0b10
#define CONTROLLER_BROADCAST    0b11

#define DATA_TYPE_U             0
#define DATA_TYPE_CR            1
#define DATA_TYPE_CRA           2
#define DATA_TYPE_CRT           3
#define DATA_TYPE_CRTTA         4
#define DATA_TYPE_CRTAAA        5
#define DATA_TYPE_TTTT          6
#define DATA_TYPE_UNDEFINED     7

#define DEVICE_RESERVED         0
#define DEVICE_BROADCAST        0b111111

/*___________________________________________________________________________*/

#define FIELD_TYPE_POS          0
#define FIELD_TYPE_LEN          2

#define FIELD_QUERY_POS         FIELD_TYPE_POS + FIELD_TYPE_LEN
#define FIELD_QUERY_LEN         1

#define FIELD_CONTROLLER_POS    FIELD_QUERY_POS + FIELD_QUERY_LEN
#define FIELD_CONTROLLER_LEN    2

#define FIELD_ID_POS            FIELD_CONTROLLER_POS + FIELD_CONTROLLER_LEN
#define FIELD_ID_LEN            6

#define FIELD_UNUSED_POS        FIELD_ID_POS + FIELD_ID_LEN
#define FIELD_UNUSED_LEN        5

#pragma pack(1)
typedef union
{
    struct
    {
        uint8_t type : FIELD_TYPE_LEN;
        uint8_t query : FIELD_QUERY_LEN;
        uint8_t controller : FIELD_CONTROLLER_LEN;
        uint8_t id : FIELD_ID_LEN;
        uint8_t unused : FIELD_UNUSED_LEN;
    } bitfields;
    uint8_t array[2];
    unsigned long value;
} can_id_t;
#pragma pack(1)

/*___________________________________________________________________________*/

#define DEVICE_ID_MODE  CAN_STDID

#define DEVICE_RXM0     
#define DEVICE_RXF0
#define DEVICE_RXF1

#define DEVICE_RXM1
#define DEVICE_RXF2
#define DEVICE_RXF3
#define DEVICE_RXF4
#define DEVICE_RXF5

/*___________________________________________________________________________*/

#define CANIOT_ERR_OK               0
#define CANIOT_ERR_DRIVER           1
#define CANIOT_ERR_NOT_INITIALIZED  2

/*___________________________________________________________________________*/

#endif