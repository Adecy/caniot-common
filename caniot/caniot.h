#include <stdint.h>

#include <mcp_can.h>

#ifndef _CANIOT_H
#define _CANIOT_H

class can_device
{
public:

    uint32_t m_id = 0;

    can_device(void);
};

#endif