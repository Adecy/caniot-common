#ifndef _CANIOT_DATA_H
#define _CANIOT_DATA_H

/*___________________________________________________________________________*/

#include <stdint.h>

#include "defines.h"

#include <avr/pgmspace.h>

/*___________________________________________________________________________*/

#define AS(buffer, struct) ((struct*)buffer)

struct contacts_set_t
{
    uint8_t c1: 1;
    uint8_t c2: 1;
    uint8_t c3: 1;
    uint8_t c4: 1;
    uint8_t c5: 1;
    uint8_t c6: 1;
    uint8_t c7: 1;
    uint8_t c8: 1;
};

struct relays_set_t
{
    uint8_t r1: 1;
    uint8_t r2: 1;
    uint8_t r3: 1;
    uint8_t r4: 1;
    uint8_t r5: 1;
    uint8_t r6: 1;
    uint8_t r7: 1;
    uint8_t r8: 1;
};

typedef int16_t temperature;

/*___________________________________________________________________________*/

struct CR_t
{
    contacts_set_t contacts;
    relays_set_t relays;
};

struct CRA_t
{
    contacts_set_t contacts;
    relays_set_t relays;
    uint16_t analog: 10;
};

struct CRT_t
{
    contacts_set_t contacts;
    relays_set_t relays;
    int16_t temperature;
};

struct TTTT_t
{
    int16_t temperatures[4];
};

/*___________________________________________________________________________*/

#define CANIOT_GET_LEN(len , type) (len = get_data_type_size(data_type_t::type))

/*___________________________________________________________________________*/

const uint8_t get_data_type_size(data_type_t dt);

/*___________________________________________________________________________*/

#endif