#include "data.h"

static const uint8_t data_type_size[] PROGMEM = {
    8u,
    1u,
    4u,
    4u,
    8u,
    8u,
    8u,
    0u,
};

const uint8_t get_data_type_size(data_type_t dt)
{
    if (dt <= ARRAY_SIZE(data_type_size)) {
        return (uint8_t)pgm_read_byte(&data_type_size[dt]);
    } else {
        return 0;
    }
}

// PROGMEM