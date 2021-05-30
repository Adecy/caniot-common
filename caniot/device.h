
#ifndef _DEVICE_H
#define _DEVICE_H

#include "caniot.h"

#include "utils.h"
#include "uart.h"

/*___________________________________________________________________________*/

extern mcp2515_can can;

/*___________________________________________________________________________*/

class can_device
{
protected:
    typedef struct
    {
        const char name[32] PROGMEM;
        const uint8_t id PROGMEM;
        const uint8_t type PROGMEM;
        const uint16_t version PROGMEM;

    } identification_t;

    typedef struct 
    {
        uint32_t uptime;
        uint8_t battery;
    } system_t;

    typedef struct
    {
        uint32_t frequency;
    } telemetry_config_t;
    

/*___________________________________________________________________________*/

public:
    static const identification_t identification;
    system_t system = {0};
    telemetry_config_t telemetry = {0};

    mcp2515_can * p_can;

    uint8_t m_ext_int_pin;
    uint32_t m_speedset;
    uint8_t m_clockset;

    uint8_t m_error = CANIOT_ERR_NOT_INITIALIZED;

    volatile uint8_t flag_can;

    uint8_t buffer[8];
    uint8_t len;
    can_id_t id;

/*___________________________________________________________________________*/

    static can_device* p_instance;

/*___________________________________________________________________________*/

    can_device() { }

    can_device(mcp2515_can * p_can, uint8_t ext_int_pin, uint32_t speedset, uint8_t clockset) : 
    p_can(p_can), m_ext_int_pin(ext_int_pin), m_speedset(speedset), m_clockset(clockset), flag_can(0)
    {
        p_instance = this;
    }

    static can_device* get_instance(void)
    {
        return p_instance;
    }

    uint8_t get_error() const { return m_error; }

    void initialize(void);
    void process_message(void);

    void print_identification(void);
};

/*___________________________________________________________________________*/

#endif