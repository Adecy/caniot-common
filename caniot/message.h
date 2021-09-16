#ifndef _CAN_MESSAGE_
#define _CAN_MESSAGE_

#include "defines.h"

class can_device;

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

    void set_errno(uint8_t errno);

    bool need_response(void) const;

    void clear(void);

    void prepare_response(Message& response, can_device& dev);
};

#endif