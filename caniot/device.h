
#ifndef _DEVICE_H
#define _DEVICE_H

#include "caniot.h"

#include "utils.h"
#include "uart.h"

#include "message.h"

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
    

/*___________________________________________________________________________*/

    static identification_t identification;
    
    system_t system = {0};
    telemetry_config_t telemetry = {0};

/*___________________________________________________________________________*/

    mcp2515_can * p_can;

    uint8_t m_ext_int_pin;
    uint32_t m_speedset;
    uint8_t m_clockset;

    uint8_t m_error = CANIOT_ERR_NOT_INITIALIZED;

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
        memcpy_P(&identification, (const void*)__DEVICE_IDENTIFICATION_ADDR__, sizeof(identification_t));
    }

    static can_device* get_instance(void)
    {
        return p_instance;
    }

    uint8_t get_error() const { return m_error; }

    void initialize(void);
    void process(void);

    int dispatch_request(Message &request, Message &response);

    int handle_read_attribute(uint16_t key, Message &response);
    int handle_write_attribute(uint16_t key, uint32_t value, Message &response);
    int handle_command(Message &response);

    void print_identification(void);
};

/*___________________________________________________________________________*/

#endif