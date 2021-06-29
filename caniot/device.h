
#ifndef _DEVICE_H
#define _DEVICE_H

#include "caniot.h"

#include "utils.h"
#include "uart.h"

#include "message.h"
#include "timer2.h"
#include "data.h"

/*___________________________________________________________________________*/

#define LOOPBACK_IF_ERR 0

#define CANIOT_DRIVER_RETRY_DELAY_MS    1000

/*___________________________________________________________________________*/
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

/*___________________________________________________________________________*/

extern identification_t __device_identification__;  // get identification location in FLASH from linker script

/*___________________________________________________________________________*/

class can_device
{
public:
    typedef struct 
    {
        uint32_t uptime;
        uint32_t abstime;
        uint32_t uptime_shift;
        uint32_t last_telemetry;
        uint32_t message_sent;
        uint32_t message_received;
        uint8_t battery;
    } system_t;

    typedef struct
    {
        uint8_t days;
        uint16_t time;
        uint8_t state;
    } schedule_t;

    typedef struct
    {
        uint32_t telemetry_period;
        schedule_t schedules[5];
    } config_t;

    identification_t identification;
    system_t system;

/*___________________________________________________________________________*/

    identification_t *p_identification = &identification;       // RAM (copied from flash)
    system_t *p_system = &system;                               // RAM
    config_t *p_config = 0;                                     // EEPROM
    schedule_t *p_schedules = (schedule_t*) sizeof(config_t);   // EEPROM

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

    uint8_t m_error = CANIOT_ENOINIT;

/*___________________________________________________________________________*/

    Message request;
    Message response;

/*___________________________________________________________________________*/

    static can_device* p_instance;

/*___________________________________________________________________________*/

    can_device() { };

    can_device(mcp2515_can * p_can, uint8_t ext_int_pin, uint32_t speedset, uint8_t clockset) : 
    p_can(p_can), m_ext_int_pin(ext_int_pin), m_speedset(speedset), m_clockset(clockset), flags(0),
    m_command_handler(nullptr)
    {
        p_instance = this;

        // TODO load identification in .initX section
        memcpy_P(p_identification, (const void*) &__device_identification__, sizeof(identification_t));

        // copy configuration in eeprom
    }

    static can_device* get_instance(void) { return p_instance; }

    uint32_t uptime(void) const { return p_system->uptime; }
    uint32_t abstime(void) const { return p_system->abstime + uptime() - p_system->uptime_shift; }
    uint8_t get_error() const { return m_error; }
    void set_command_handler(command_handler_t handler) { m_command_handler = handler; }
    void set_telemetry_builder(telemetry_builder_t builder) { m_telemetry_builder = builder; }

    void initialize(void);
    void process(void);

    void print_identification(void);

    void request_telemetry(void);

protected:
    void process_query(void);
    void process_telemetry(void);

    uint8_t dispatch_request(Message &request, Message &response);
    
    uint8_t read_attribute(const uint16_t key, Message &response);
    uint8_t write_attribute(const uint16_t key, const uint32_t value, Message &response);

    void prepare_error(Message &request, const uint8_t errno);
    uint8_t send_response(Message &response);    
};

/*___________________________________________________________________________*/

#endif