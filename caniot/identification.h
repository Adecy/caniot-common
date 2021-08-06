#include "caniot/device.h"

// build_unflags =
//     -Wl,--gc-sections

// TODO use .id, .type, ...
// Set identification at location 0x77dc in flash
__attribute__((used, section(".device_identification"))) static const identification_t identification = {
    {
        __DEVICE_ID__,
        __DEVICE_TYPE__,
    },
    __FIRMWARE_VERSION__,
    __DEVICE_NAME__,
};