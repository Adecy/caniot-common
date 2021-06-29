
#ifndef _ATTRIBUTES_H
#define _ATTRIBUTES_H

/*___________________________________________________________________________*/

#include <stdint.h>

#include <avr/pgmspace.h>
#include <avr/eeprom.h>

#include <utils.h>

/*___________________________________________________________________________*/

#define ATTR_IDENTIFICATION 0
#define ATTR_SYSTEM 1
#define ATTR_CONFIG 2
#define ATTR_SCHEDULES 3

#define ATTR_KEY_SECTION(key) (((uint16_t)key) >> 12)
#define ATTR_KEY_ATTR(key) ((((uint16_t)key) >> 4) & 0xF)
#define ATTR_KEY_PART(key) (((uint16_t)key) & 0xF)

#define ATTR_KEY(section, attr, part) ((section & 0xF) << 12 | (attr & 0xFF) << 4 | (part & 0xF))

#define ATTR_WRITTABLE 0
#define ATTR_READONLY 1 << 3

#define ATTR_DEFAULT ATTR_WRITTABLE

/*___________________________________________________________________________*/

typedef enum : uint8_t
{
    RAM = 1 << 0,
    EEPROM = 1 << 1,
    PROGMEMORY = 1 << 2,
    READONLY = 1 << 3,
    PRIVATE = 1 << 4, // unused
} section_option_t;

struct section_t
{
    uint8_t section;
    uint8_t options;
    char name[15];
};

struct attribute_t
{
    uint8_t section;
    uint8_t index;
    uint8_t size;
    uint8_t readonly;
    char name[20];
};

/*___________________________________________________________________________*/

static const struct section_t attributes_sections[] PROGMEM {
    {ATTR_IDENTIFICATION, RAM | PROGMEMORY | READONLY, "identification"},
    {ATTR_SYSTEM, RAM, "system"},
    {ATTR_CONFIG, EEPROM, "configuration"},
    {ATTR_SCHEDULES, EEPROM, "schedules"}
};

static const struct attribute_t attributes[] PROGMEM = {
    {ATTR_IDENTIFICATION, 0, 1, ATTR_DEFAULT, "nodeid"},
    {ATTR_IDENTIFICATION, 1, 1, ATTR_DEFAULT, "version"},
    {ATTR_IDENTIFICATION, 2, 32, ATTR_DEFAULT, "name"},

    {ATTR_SYSTEM, 0, 4, ATTR_READONLY, "uptime"},
    {ATTR_SYSTEM, 1, 4, ATTR_WRITTABLE, "abstime"},
    {ATTR_SYSTEM, 2, 4, ATTR_READONLY, "uptime_shift"},
    {ATTR_SYSTEM, 3, 4, ATTR_READONLY, "last_telemetry"},
    {ATTR_SYSTEM, 4, 4, ATTR_READONLY, "message_sent"},
    {ATTR_SYSTEM, 5, 4, ATTR_READONLY, "message_recv"},
    {ATTR_SYSTEM, 6, 1, ATTR_READONLY, "battery"},

    {ATTR_CONFIG, 0, 4, ATTR_DEFAULT, "telemetry_period"},

    {ATTR_SCHEDULES, 1, 1, ATTR_DEFAULT, "days"},
    {ATTR_SCHEDULES, 1, 1, ATTR_DEFAULT, "time"},
    {ATTR_SCHEDULES, 4, 1, ATTR_DEFAULT, "command"},
};

/*___________________________________________________________________________*/

typedef uint16_t key_t;

typedef union
{
    uint32_t u32;
    uint16_t u16;
    uint8_t u8;
} value_t;

typedef struct
{
    uint8_t section;
    section_option_t options;
    uint8_t offset;
    uint8_t size;
} attr_ref_t;

/*___________________________________________________________________________*/

#endif