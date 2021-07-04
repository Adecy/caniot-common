#ifndef _CONFIG_H_
#define _CONFIG_H_

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

typedef struct
{
    uint8_t days;
    uint16_t time;
    uint8_t state;
} switching_point_t;

typedef struct
{
    uint32_t telemetry_period;

    uint32_t _reserved[9];

    switching_point_t schedule[5];
} config_t;

class Configuration
{
public:
    config_t data;  // EEMEM

    Configuration(void) { }

    uint32_t get_telemetry_period(void) const
    {
        return (uint32_t) eeprom_read_dword(&data.telemetry_period);
    }

    void get_switching_point(uint8_t idx, switching_point_t *sp_p) const
    {
        eeprom_read_block(sp_p, &data.schedule[idx], sizeof(switching_point_t));
    }
};

#endif