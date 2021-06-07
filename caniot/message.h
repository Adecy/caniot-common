#ifndef _CAN_MESSAGE_
#define _CAN_MESSAGE_

#include "defines.h"

class Message
{
public:
    can_id_t id;
    uint8_t len;
    uint8_t buffer[8];

    Message(void)
    {
        id.value = 0;
        len = 0;
        memset(buffer, 0x00, sizeof(buffer));
    }

    bool is_query(void)
    {
        return id.bitfields.query == 0x1;
    }

    bool is_broadcast(void)
    {
        return (id.bitfields.device_type << 3 | id.bitfields.device_id) == DEVICE_BROADCAST;
    }

    type_t get_type(void)
    {
        return (type_t) id.bitfields.device_type;
    }
};

#endif