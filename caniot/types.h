#ifndef _CANIOT_TYPES_H
#define _CANIOT_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef struct // 36 B
{
    union
    {
        struct
        {
            uint8_t id : 3;
            uint8_t type : 3;
        } device;
        uint8_t deviceid;
    };
    uint16_t version;
    char name[32];

} identification_t;

typedef struct
{
    uint32_t uptime;
    uint32_t abstime;
    uint32_t calculated_abstime;
    uint32_t uptime_shift;
    uint32_t last_telemetry;
    struct stats_t
    {
        struct received_t
        {
            uint32_t total;
            uint32_t read_attribute;
            uint32_t write_attribute;
            uint32_t command;
            uint32_t request_telemetry;
            uint32_t processed;
            uint32_t query_failed;
        } received;
        struct sent_t
        {
            uint32_t total;
            uint32_t telemetry;
        } sent;
        struct events_t {
            uint32_t total;
        } events;
    } stats;
    uint8_t last_query_error;
    uint8_t last_telemetry_error;
    uint8_t last_event_error;
    uint8_t battery;
} system_t;

#endif