#include "message.h"

#include "device.h"

void Message::set_errno(uint8_t errno)
{
    len = 1u;
    buffer[0] = errno;

    id.bitfields.query = query_t::response;
    id.bitfields.type = type_t::command;
}

bool Message::need_response(void) const
{
    return ((id.value & 0b111) != (FRAME_COMMAND | FRAME_QUERY << 2)) &&
        ((id.value & 0b111) != (FRAME_TELEMETRY | FRAME_QUERY << 2));
}

void Message::clear(void)
{
    len = 0;
    memset(buffer, 0x00, sizeof(buffer));
}

void Message::prepare_response(Message& response, can_device& dev)
{
    // assert is_query()

    response.clear();
    response.id.value = BUILD_ID(
        id.bitfields.type,
        query_t::response,
        id.bitfields.controller,
        dev.identification.device.type,
        dev.identification.device.id
    );
}
