#include "device.h"

#include <errno.h>


/*___________________________________________________________________________*/

// Set identification at location 0x77dd in flash, at ther very end of the flash (on ATmega328p)
__attribute__((used, section(".device_identification"))) static const identification_t device_identification = {
    {
        __DEVICE_ID__,
        __DEVICE_TYPE__,
    },
    __FIRMWARE_VERSION__,
    __DEVICE_NAME__,
};

/*___________________________________________________________________________*/

can_device *can_device::p_instance;

can_device::can_device(mcp2515_can *p_can, uint8_t ext_int_pin, uint32_t speedset, uint8_t clockset) : p_can(p_can), m_ext_int_pin(ext_int_pin), m_speedset(speedset), m_clockset(clockset), flags(0),
                                                                                                       m_command_handler(nullptr)
{
    p_instance = this;

    memcpy_P(&identification, (const void *)&device_identification, sizeof(identification_t));

    // copy configuration in eeprom if not set
}

/*___________________________________________________________________________*/

ISR(INT0_vect)
{
    SET_FLAG_COMMAND(can_device::get_instance()->flags);
}

void can_device::initialize(void)
{
    while (CAN_OK != p_can->begin(m_speedset, m_clockset))
    {
        _delay_ms(CANIOT_DRIVER_RETRY_DELAY_MS);

        m_error = CANIOT_EDRIVER;
    };

    m_error = CANIOT_OK;

    uint8_t err = MCP2515_OK;
    err |= p_can->init_Mask(0, CAN_STDID, DEVICE_RXM0);
    err |= p_can->init_Filt(0, CAN_STDID, DEVICE_RXF0);
    err |= p_can->init_Filt(1, CAN_STDID, DEVICE_RXF1);

    err |= p_can->init_Mask(1, CAN_STDID, DEVICE_RXM1);
    err |= p_can->init_Filt(2, CAN_STDID, DEVICE_RXF2);
    err |= p_can->init_Filt(3, CAN_STDID, DEVICE_RXF3);
    err |= p_can->init_Filt(4, CAN_STDID, DEVICE_RXF4);
    err |= p_can->init_Filt(5, CAN_STDID, DEVICE_RXF5);

    if (err != CANIOT_OK)
    {
        static const char failed_init[] PROGMEM = "failed to initialize mcp2515";
        usart_printl_p(failed_init);

        m_error = CANIOT_EDRIVER;

        exit(m_error);
    }

    // interrupt when receiving a can message
    // falling on INT0
    EICRA |= 1 << ISC01;
    EICRA &= ~(1 << ISC00);
    EIMSK |= 1 << INTF0;

    timer2_init();
}

void can_device::process(void)
{
    // update uptime
    timer2_uptime(&system.uptime);
    system.calculated_abstime = abstime();

    // calculation of quantities
    // TODO do to it less often
    system.battery = battery();

    const uint32_t telemetry_period = config.get_telemetry_period();
    if (telemetry_period && (uptime() - system.last_telemetry >= telemetry_period))
    {
        SET_FLAG_TELEMETRY(flags);
    }

    if (TEST_FLAG_COMMAND(flags))
    {
        system.last_query_error = process_query();
    }

    if (TEST_FLAG_TELEMETRY(flags))
    {
        CLEAR_FLAG_TELEMETRY(flags);

        system.last_telemetry_error = process_telemetry();
    }
}

uint8_t can_device::process_query(void)
{
    uint8_t err = CAN_OK;
    if (CAN_MSGAVAIL == p_can->checkReceive())
    {
        p_can->readMsgBufID(&request.id.value, &request.len, request.buffer);
        system.stats.received.total++;

#if LOG_LEVEL_INFO
        print_can_expl(request);
#endif

        memset(response.buffer, 0x00, 8);
        err = dispatch_request(request, response);

#if LOG_LEVEL_DBG
        usart_hex(err);
        usart_printl(" = dispatch_request error");
#endif

        // success
        if (err == CANIOT_OK)
        {   
            system.stats.received.processed++;
            if (request.need_response())
            {
                err = send_response(response);
            }
        } 
        else
        {
            // loopback if error
            system.stats.received.query_failed++;
            if (LOOPBACK_IF_ERR)
            {
                err = send_response(request);
            }
            else // return error frame
            {
                response.set_errno(err);
                err = send_response(response);
            }
        }
    }
    else
    {
        CLEAR_FLAG_COMMAND(flags);
    }

#if LOG_LEVEL_ERROR
    // handle errors
    if (err != CANIOT_OK)
    {
        usart_print("Error command handling : ");
        usart_hex(err);
        usart_transmit('\n');
    }
#endif

    return err;
}

uint8_t can_device::process_telemetry(void)
{
    uint8_t err = CAN_OK;

    if (nullptr != m_telemetry_builder)
    {
        system.last_telemetry = system.uptime;

        memset(response.buffer, 0x00, 8);
        err = m_telemetry_builder(response.buffer, response.len);
        if (err == CANIOT_OK)
        {
            // length must at least contain the data_type, it may contain more information
            if (response.len >= get_data_type_size((data_type_t)__DEVICE_TYPE__))
            {
                system.stats.sent.telemetry++;

                response.id.value = BUILD_ID(type_t::telemetry, query_t::response, controller_t::broadcast, __DEVICE_TYPE__, __DEVICE_ID__);
                
                err = send_response(response);
            }
            else
            {
                // error failed to send telemetry message
                err = CANIOT_ETELEMETRY;

                // don't retry immediately (can cause loop)
            }
        }
    }
    else
    {
        err = CANIOT_EHANDLERT;
    }

#if LOG_LEVEL_ERROR
    // handle errors
    if (err != CANIOT_OK)
    {
        usart_print("Error telemetry : ");
        usart_hex(err);
        usart_transmit('\n');
    }
#endif

    return err;
}

uint8_t can_device::dispatch_request(Message &request, Message &response)
{
    uint8_t ret = CANIOT_ENPROC;
    if (request.is_query())
    {
        response.id.value = request.id.value;
        response.id.bitfields.query = query_t::response;
        response.len = 0u;  // default len

        switch (request.get_type())
        {
        case type_t::command:
        {
            const data_type_t dt = request.get_data_type();
            if ((dt == identification.device.type) && (request.len == get_data_type_size(dt)))
            {
                if (m_command_handler != nullptr)
                {
                    ret = m_command_handler(request.buffer, request.len);
                    if (ret == CANIOT_OK)
                    {
                        SET_FLAG_TELEMETRY(flags);
                        system.stats.received.command++;
                    }
                }
                else
                {
                    ret = CANIOT_EHANDLERC;
                }
            }
            else
            {
                ret = CANIOT_ECMD;
            }
        }
        break;

        case type_t::read_attribute:
            if (request.len == 2u)
            {
                const key_t key = *(key_t *)request.buffer;
                attr_ref_t attr_ref;
                ret = Attributes::resolve(key, &attr_ref);
                if (ret == CANIOT_OK)
                {
                    ret = Attributes::read(&attr_ref, (value_t *)&response.buffer[2]);
                    if (ret == CANIOT_OK)
                    {
                        response.len = 6u;
                        *(key_t *)response.buffer = key; // copy key
                        system.stats.received.read_attribute++;
                    }
                }
            }
            break;

        case type_t::write_attribute:
            if ((request.len >= 3) && (request.len <= 6))
            {
                const key_t key = *(key_t *)request.buffer;
                attr_ref_t attr_ref;
                ret = Attributes::resolve(key, &attr_ref);
                if (ret == CANIOT_OK)
                {
                    ret = Attributes::write(&attr_ref, *(value_t *)&request.buffer[2]);
                    if (ret == CANIOT_OK)
                    {
                        // handle special cases
                        if (key == KEY_ATTR_ABSTIME)
                        {
                            system.uptime_shift = system.uptime;
#if LOG_LEVEL_DBG
                            usart_printl("update uptime_shift");
#endif
                        }

                        // TODO : maybe read is not necessary
                        ret = Attributes::read(&attr_ref, (value_t *)&response.buffer[2]);
                        if (ret == CANIOT_OK)
                        {
                            response.len = 6u;
                            *(key_t *)response.buffer = key; // copy key
                            system.stats.received.write_attribute++;
                        }
                    }
                }
            }
            break;

        case type_t::telemetry:
            request_telemetry();
            system.stats.received.request_telemetry++;
            ret = CANIOT_OK;
            break;
            
        default:
            ret = CANIOT_ENPROC; // error unprocessable request type
        }
    }
    return ret;
}


uint8_t can_device::send_response(Message &response)
{
#if LOG_LEVEL_INFO
    print_can_expl(response);
#endif

    system.stats.sent.total++;

    return p_can->sendMsgBuf(response.id.value, CAN_STDID, response.len, response.buffer);
}

void can_device::request_telemetry(void)
{
    SET_FLAG_TELEMETRY(flags);
}

const uint8_t can_device::battery(void) const
{
    return MAINS_POWER_SUPPLY;
}

/*___________________________________________________________________________*/

void can_device::print_identification(void)
{
    usart_print("name    = ");
    usart_printl(identification.name);

    usart_print("id      = ");
    usart_hex(identification.device.id);
    usart_transmit('\n');

    usart_print("type    = ");
    usart_hex(identification.device.type);
    usart_transmit('\t');

    print_prog_data_type((data_type_t) identification.device.type);

    usart_print("\nversion = ");
    usart_u16(identification.version);
    usart_transmit('\n');
}

/*___________________________________________________________________________*/