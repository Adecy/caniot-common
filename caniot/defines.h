#ifndef _CANIOT_DEF_H
#define _CANIOT_DEF_H

#include <stdint.h>

#include <mcp2515_can_dfs.h>

/*___________________________________________________________________________*/

#define FRAME_COMMAND           0b00
#define FRAME_TELEMETRY         0b01
#define FRAME_WRITE_ATTRIBUTE   0b10
#define FRAME_READ_ATTRIBUTE    0b11

typedef enum : uint8_t
{
    command = FRAME_COMMAND,
    telemetry = FRAME_TELEMETRY,
    write_attribute = FRAME_WRITE_ATTRIBUTE,
    read_attribute = FRAME_READ_ATTRIBUTE,
} type_t;

#define FRAME_QUERY             0b0
#define FRAME_RESPONSE          0b1

typedef enum : uint8_t
{
    query = FRAME_QUERY,
    response = FRAME_RESPONSE,
} query_t;

#define CONTROLLER_MAIN         0b00
#define CONTROLLER_1            0b01
#define CONTROLLER_2            0b10
#define CONTROLLER_BROADCAST    0b11

typedef enum : uint8_t
{
    main_controller = CONTROLLER_MAIN,
    controller1 = CONTROLLER_1,
    controller2 = CONTROLLER_2,
    broadcast = CONTROLLER_BROADCAST
} controller_t;

#define DATA_TYPE_U             0
#define DATA_TYPE_CR            1
#define DATA_TYPE_CRA           2
#define DATA_TYPE_CRT           3
#define DATA_TYPE_CRTTA         4
#define DATA_TYPE_CRTAAA        5
#define DATA_TYPE_TTTT          6
#define DATA_TYPE_UNDEFINED     7

typedef enum : uint8_t
{
    U = DATA_TYPE_U,
    CR = DATA_TYPE_CR,
    CRA = DATA_TYPE_CRA,
    CRT = DATA_TYPE_CRT,
    CRTTA = DATA_TYPE_CRTTA,
    CRTAAA = DATA_TYPE_CRTAAA,
    TTTT = DATA_TYPE_TTTT,
    UNDEFINED = DATA_TYPE_UNDEFINED,
} data_type_t;

#define DEVICE_RESERVED         0
#define DEVICE_BROADCAST        0b111111

/*___________________________________________________________________________*/

#define FIELD_TYPE_POS              0
#define FIELD_TYPE_LEN              2

#define FIELD_QUERY_POS             FIELD_TYPE_POS + FIELD_TYPE_LEN
#define FIELD_QUERY_LEN             1

#define FIELD_CONTROLLER_POS        FIELD_QUERY_POS + FIELD_QUERY_LEN
#define FIELD_CONTROLLER_LEN        2

#define FIELD_DEVICE_TYPE_POS       FIELD_CONTROLLER_POS + FIELD_CONTROLLER_LEN
#define FIELD_DEVICE_TYPE_LEN       3

#define FIELD_DEVICE_ID_POS         FIELD_DEVICE_TYPE_POS + FIELD_DEVICE_TYPE_LEN
#define FIELD_DEVICE_ID_LEN         3

#define FIELD_UNUSED_POS            FIELD_DEVICE_ID_POS + FIELD_DEVICE_ID_LEN
#define FIELD_UNUSED_LEN            5

#pragma pack(1)
typedef union
{
    struct
    {
        uint8_t type : FIELD_TYPE_LEN;
        uint8_t query : FIELD_QUERY_LEN;
        uint8_t controller : FIELD_CONTROLLER_LEN;
        uint8_t device_type : FIELD_DEVICE_TYPE_LEN;
        uint8_t device_id : FIELD_DEVICE_ID_LEN;
        uint8_t : FIELD_UNUSED_LEN;
        uint16_t : 16;
    } bitfields;
    uint8_t array[2][2];
    unsigned long value;
    
    bool is_broadcast(void) const
    {
        return (bitfields.device_type << 3 | bitfields.device_id) == DEVICE_BROADCAST;
    }

} can_id_t;
#pragma pack(1)

/*___________________________________________________________________________*/

#define BUILD_ID(type, qr, controller, devicetype, deviceid) (type | qr << 2 | controller << 3 | devicetype << 5 | deviceid << 8)

#define HAS_RESPONSE_TO_REQUEST(id) ((id & 0b111) != (FRAME_COMMAND | FRAME_QUERY << 2))

/*___________________________________________________________________________*/

#define DEVICE_ID_MODE  CAN_STDID

#define DEVICE_RXM0     0b11111100100
// #define DEVICE_RXF0     0x300
// #define DEVICE_RXF1     0x300
// #define DEVICE_RXF0     0 | (0b111111 << 5)
#define DEVICE_RXF0     (FRAME_QUERY << 2) | (__DEVICE_TYPE__ << 5) | (__DEVICE_ID__ << 8)
#define DEVICE_RXF1     DEVICE_RXF0

#define DEVICE_RXM1     DEVICE_RXM0
#define DEVICE_RXF2     (FRAME_QUERY << 2) | 0b111111 << 5
#define DEVICE_RXF3     DEVICE_RXF2
#define DEVICE_RXF4     DEVICE_RXF3
#define DEVICE_RXF5     DEVICE_RXF4

/*___________________________________________________________________________*/

#define CANIOT_OK          0x00         // OK
#define CANIOT_ENPROC      0x01         // ERROR UNPROCESSABLE
#define CANIOT_ECMD        0x02         // ERROR COMMAND
#define CANIOT_EKEY        0x03         // ERROR KEY (read/write-attribute)
#define CANIOT_ETIMEOUT    0x04         // ERROR TIMEOUT
#define CANIOT_EBUSY       0x05         // ERROR BUSY
#define CANIOT_EFMT        0x06         // ERROR FORMAT
#define CANIOT_EHANDLERC   0x07         // ERROR UNDEFINED COMMAND HANDLER 
#define CANIOT_EHANDLERT   0x08         // ERROR UNDEFINED TELEMETRY HANDLER
#define CANIOT_ETELEMETRY  0x09         // ERROR TELEMETRY

#define CANIOT_ENOINIT     0x10         // ERROR NOT INITIALIZED
#define CANIOT_EDRIVER     0x11         // ERROR DRIVER

#define CANIOT_ENIMPL      0xFE         // ERROR NOT IMPLEMENTED

#define CANIOT_ERROR       0xFF         // ANY ERROR + -1

/*___________________________________________________________________________*/

#define CONFIG_TELEMETRY_DISABLE    0

/*___________________________________________________________________________*/

#endif