
#ifndef _DEVICE_H
#define _DEVICE_H

#include "caniot.h"

#include "utils.h"
#include "uart.h"

#include "message.h"

#include "timer2.h"

/*___________________________________________________________________________*/

#define LOOPBACK_IF_ERR 1

/*___________________________________________________________________________*/

extern mcp2515_can can;

/*___________________________________________________________________________*/

class can_device
{
public:
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
        uint32_t uptime_shift;
        uint32_t message_sent;
        uint32_t message_received;
        uint8_t battery;
    } system_t;

    typedef struct
    {
        uint32_t period;
    } telemetry_config_t;

    typedef struct
    {
        struct {
            uint8_t days;
            uint16_t time;
            uint8_t state;
        } schedules[5];
    } schedule_t;

    struct attributes
    {
        identification_t identification;
        system_t system;
        telemetry_config_t period;
        schedule_t schedules;
    };
    

/*___________________________________________________________________________*/

    static struct attributes attributes;

    identification_t *p_identification = &attributes.identification;
    system_t *p_system = &attributes.system;
    telemetry_config_t *p_period = &attributes.period;
    schedule_t *p_schedules = &attributes.schedules;

/*___________________________________________________________________________*/

    mcp2515_can * p_can;

    uint8_t m_ext_int_pin;
    uint32_t m_speedset;
    uint8_t m_clockset;

    uint8_t m_error = CANIOT_ENOINIT;

    volatile uint8_t flag_can;

/*___________________________________________________________________________*/

    Message request;
    Message response;

/*___________________________________________________________________________*/

    static can_device* p_instance;

/*___________________________________________________________________________*/

    can_device() { };

    can_device(mcp2515_can * p_can, uint8_t ext_int_pin, uint32_t speedset, uint8_t clockset) : 
    p_can(p_can), m_ext_int_pin(ext_int_pin), m_speedset(speedset), m_clockset(clockset), flag_can(0)
    {
        p_instance = this;

        // todo load identification in .initX section
        memcpy_P(p_identification, (const void*)__DEVICE_IDENTIFICATION_ADDR__, sizeof(identification_t));
    }

    static can_device* get_instance(void)
    {
        return p_instance;
    }

    uint32_t uptime(void) const
    {
        return p_system->uptime;
    }

    uint32_t abstime(void) const
    {
        return p_system->abstime + uptime() - p_system->uptime_shift;
    }

    uint8_t get_error() const { return m_error; }

    void initialize(void);
    void process(void);

    uint8_t dispatch_request(Message &request, Message &response);
    void prepare_error(Message &request, const uint8_t errno);

    uint8_t handle_command(const data_type_t data_type, Message &response);
    uint8_t read_attribute(const uint16_t key, Message &response);
    uint8_t write_attribute(const uint16_t key, const uint32_t value, Message &response);

    uint8_t send_response(Message &response);

    void print_identification(void);
};

/*___________________________________________________________________________*/

#endif