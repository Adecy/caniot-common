
#ifndef _DEVICE_H
#define _DEVICE_H

#include "caniot.h"

#include "tqueue.h"
#include "misc.h"
#include "uart.h"

#include "message.h"
#include "timer2.h"
#include "data.h"
#include "attributes.h"
#include "config.h"
#include "types.h"
#include "scheduler.h"

/*___________________________________________________________________________*/

#define LOOPBACK_IF_ERR 0

#define CANIOT_DRIVER_RETRY_DELAY_MS    1000

/*___________________________________________________________________________*/

class can_device
{
public:

/*___________________________________________________________________________*/

    identification_t identification;
    system_t system;
    Configuration config;       

/*___________________________________________________________________________*/

    mcp2515_can * p_can;

    uint8_t m_ext_int_pin;  // ignored, default 0
    uint32_t m_speedset;
    uint8_t m_clockset;

    #define SET_FLAG(flags, bit) (flags |= 1 << bit)     
    #define SET_FLAG_COMMAND(flags) SET_FLAG(flags, 0)
    #define SET_FLAG_TELEMETRY(flags) SET_FLAG(flags, 1)

    #define CLEAR_FLAG(flags, bit) (flags &= ~(1 << bit))
    #define CLEAR_FLAG_COMMAND(flags) CLEAR_FLAG(flags, 0)
    #define CLEAR_FLAG_TELEMETRY(flags) CLEAR_FLAG(flags, 1)

    #define TEST_FLAG(flags, bit) ((flags & (1 << bit)) != 0)
    #define TEST_FLAG_COMMAND(flags) TEST_FLAG(flags, 0)
    #define TEST_FLAG_TELEMETRY(flags) TEST_FLAG(flags, 1)

    volatile uint8_t flags;

/*___________________________________________________________________________*/

    typedef uint8_t (*command_handler_t)(uint8_t buffer[8], uint8_t len);
    typedef uint8_t (*telemetry_builder_t)(uint8_t buffer[8], uint8_t &len);

    command_handler_t m_command_handler;
    telemetry_builder_t m_telemetry_builder;

    uint8_t m_error = CANIOT_ENOINIT;   // use this error in attributes

/*___________________________________________________________________________*/

    Message request;
    Message response;

/*___________________________________________________________________________*/

    static can_device* p_instance;

/*___________________________________________________________________________*/

    can_device() { };

    can_device(mcp2515_can* p_can, uint8_t ext_int_pin, uint32_t speedset, uint8_t clockset);

    static can_device* get_instance(void) { return p_instance; }

    uint32_t uptime(void) const { return system.uptime; }
    uint32_t abstime(void) const { return system.abstime + uptime() - system.uptime_shift; }
    uint8_t get_error() const { return m_error; }
    void set_command_handler(command_handler_t handler) { m_command_handler = handler; }
    void set_telemetry_builder(telemetry_builder_t builder) { m_telemetry_builder = builder; }

    void initialize(void);
    void process(void);

    void print_identification(void);

/*___________________________________________________________________________*/

    virtual const uint8_t battery(void) const;
    
/*___________________________________________________________________________*/

protected:
    uint8_t process_query(void);
    uint8_t process_telemetry(void);

    uint8_t dispatch_request(Message &request, Message &response);

    uint8_t handle_command(Message &request);
    uint8_t handle_read_attribute(Message &request, Message &response);
    uint8_t handle_write_attribute(Message &request, Message &response);
    uint8_t handle_request_telemetry(void);

    void prepare_error(Message &request, const uint8_t errno);
    uint8_t send_response(Message &response);    
};

/*___________________________________________________________________________*/

#endif