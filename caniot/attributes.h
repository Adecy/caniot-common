
#ifndef _ATTRIBUTES_H
#define _ATTRIBUTES_H

#include <stdint.h>

#include <avr/pgmspace.h>

// extern uint8_t caniot_attributes;

struct attribute_ref {
    uint16_t key;
    uint8_t size;
    char name[12];
};

const struct attribute_ref attributes[] PROGMEM = {
    {0x0000u, 1, "nodeid"},
    {0x0001u, 2, "version"},
    {0x0003u, 32, "name"},

    {0x0100u, 4, "uptime"},
    {0x0101u, 4, "abstime"},
    {0x0102u, 4, "uptimeshift"},
    {0x0103u, 4, "messagesent"},
    {0x0104u, 4, "messegerecv"},
    {0x0105u, 1, "battery"},

    {0x0200u, 4, "telemperiod"},
};

#endif