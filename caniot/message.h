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
        clear();
    }

    bool is_query(void) const
    {
        return id.is_query();
    }

    inline bool is_broadcast(void) const
    {
        return id.is_broadcast();
    }

    type_t get_type(void) const
    {
        return (type_t)id.bitfields.type;
    }

    data_type_t get_data_type(void) const
    {
        return (data_type_t)id.bitfields.device_type;
    }

    void set_errno(uint8_t errno)
    {
        len = 1u;
        buffer[0] = errno;

        id.bitfields.query = query_t::response;
        id.bitfields.type = type_t::command;
    }

    bool need_response(void) const
    {
        return ((id.value & 0b111) != (FRAME_COMMAND | FRAME_QUERY << 2)) &&
            ((id.value & 0b111) != (FRAME_TELEMETRY | FRAME_QUERY << 2));
    }

    void clear(void) 
    {
        len = 0;
        memset(&id, 0x00, sizeof(can_id_t));
        memset(buffer, 0x00, sizeof(buffer));
    }
};

#endif