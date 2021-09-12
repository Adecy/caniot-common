#include "utils.h"

/*___________________________________________________________________________*/

uint8_t xor_checksum(void* const data, const size_t size)
{
    uint8_t checksum = 0xFF;
    for (size_t i = 0; i < size; i++) {
        checksum ^= *((uint8_t*)data + i);
    }
    return checksum;
}

/*___________________________________________________________________________*/