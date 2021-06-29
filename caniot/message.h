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
        return id.bitfields.query == query_t::query;
    }

    inline bool is_broadcast(void) const
    {
        return id.is_broadcast();
    }

    type_t get_type(void) const 
    {
        return (type_t) id.bitfields.type;
    }

    data_type_t get_data_type(void) const
    {
        return (data_type_t) id.bitfields.device_type;
    }

    void set_errno(uint8_t errno)
    {
        len = 1u;
        buffer[0] = errno;
        id.bitfields.query = 0;
        id.bitfields.type = 0;
    }

    bool need_response(void) const
    {
        return ((id.value & 0b111) != (FRAME_COMMAND | FRAME_QUERY << 2)) &&
               ((id.value & 0b111) != (FRAME_TELEMETRY | FRAME_QUERY << 2));
    }
};

#endif