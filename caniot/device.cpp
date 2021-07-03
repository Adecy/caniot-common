#include "device.h"

#include <errno.h>

/*___________________________________________________________________________*/

can_device *can_device::p_instance;

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
    timer2_uptime(&p_system->uptime);
    p_system->calculated_abstime = abstime();

    // calculation of quantities
    // TODO do to it less often
    p_system->battery = battery();

    if (p_config->telemetry_period && (uptime() - p_system->last_telemetry >= p_config->telemetry_period))
    {
        SET_FLAG_TELEMETRY(flags);
    }

    if (TEST_FLAG_COMMAND(flags))
    {
        p_system->last_query_error = process_query();
    }

    if (TEST_FLAG_TELEMETRY(flags))
    {
        p_system->last_telemetry_error = process_telemetry();
    }
}

uint8_t can_device::process_query(void)
{
    uint8_t err = CAN_OK;
    if (CAN_MSGAVAIL == p_can->checkReceive())
    {
        p_can->readMsgBufID(&request.id.value, &request.len, request.buffer);
        p_system->stats.received.total++;

#if LOG_LEVEL_INFO
        print_can_expl(request);
#endif

        memset(response.buffer, 0x00, 8);
        err = dispatch_request(request, response);

#if LOG_LEVEL_DBG
        usart_u8(err);
        usart_printl(" = dispatch_request error");
#endif

        // success
        if (err == CANIOT_OK)
        {   
            p_system->stats.received.processed++;
            if (request.need_response())
            {
                err = send_response(response);
            }
        } 
        else
        {
            // loopback if error
            p_system->stats.received.query_failed++;
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
        p_system->last_telemetry = p_system->uptime;

        memset(response.buffer, 0x00, 8);
        err = m_telemetry_builder(response.buffer, response.len);
        if (err == CANIOT_OK)
        {
            // length must be at least biffer, it may contain more information
            if (response.len >= get_data_type_size((data_type_t)__DEVICE_TYPE__))
            {
                p_system->stats.sent.telemetry++;

                response.id.value = BUILD_ID(type_t::telemetry, query_t::response, controller_t::broadcast, __DEVICE_TYPE__, __DEVICE_ID__);
                
                err = send_response(response);
            }
            else
            {
                // error failed to send telemetry message
                err = CANIOT_ETELEMETRY;
            }
        }

        CLEAR_FLAG_TELEMETRY(flags);
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
            if ((dt == p_identification->device.type) && (request.len == get_data_type_size(dt)))
            {
                if (m_command_handler != nullptr)
                {
                    ret = m_command_handler(request.buffer, request.len);
                    if (ret == CANIOT_OK)
                    {
                        SET_FLAG_TELEMETRY(flags);
                        p_system->stats.received.command++;
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
                ret = resolve_attribute(key, &attr_ref);
                if (ret == CANIOT_OK)
                {
                    ret = read_attribute(&attr_ref, (value_t *)&response.buffer[2]);
                    if (ret == CANIOT_OK)
                    {
                        response.len = 6u;
                        *(key_t *)response.buffer = key; // copy key
                        p_system->stats.received.read_attribute++;
                    }
                }
            }
            break;

        case type_t::write_attribute:
            if ((request.len >= 3) && (request.len <= 6))
            {
                const key_t key = *(key_t *)request.buffer;
                attr_ref_t attr_ref;
                ret = resolve_attribute(key, &attr_ref);
                if (ret == CANIOT_OK)
                {
                    ret = write_attribute(&attr_ref, *(value_t *)&request.buffer[2]);
                    if (ret == CANIOT_OK)
                    {
                        // handle special cases
                        if (key == KEY_ATTR_ABSTIME)
                        {
                            p_system->uptime_shift = p_system->uptime;
#if LOG_LEVEL_DBG
                            usart_printl("upated uptime_shift");
#endif
                        }

                        // TODO : maybe read is not necessary
                        ret = read_attribute(&attr_ref, (value_t *)&response.buffer[2]);
                        if (ret == CANIOT_OK)
                        {
                            response.len = 6u;
                            *(key_t *)response.buffer = key; // copy key
                            p_system->stats.received.write_attribute++;
                        }
                    }
                }
            }
            break;

        case type_t::telemetry:
            request_telemetry();
            p_system->stats.received.request_telemetry++;
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

    p_system->stats.sent.total++;

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

const uint8_t can_device::resolve_attribute(const key_t key, attr_ref_t *const p_attr_ref)
{
    if (ATTR_KEY_SECTION(key) < ARRAY_SIZE(attributes_sections))
    {
        const section_t *section_p = &attributes_sections[ATTR_KEY_SECTION(key)];
        uint8_t attr_index = 0;
        uint8_t offset = 0;

        for (uint_fast8_t i = 0; i < ARRAY_SIZE(attributes); i++)
        {
            const uint8_t section = pgm_read_byte(&section_p->section);
            if (pgm_read_byte(&attributes[i].section) == section)
            {
                const uint8_t attr_size = pgm_read_byte(&attributes[i].size);
                if (attr_index == ATTR_KEY_ATTR(key))
                {
                    const uint8_t attr_offset = ATTR_KEY_PART(key) << 2;
                    if (attr_offset < attr_size)
                    {
                        const section_option_t option = (section_option_t)(pgm_read_byte(&section_p->options) |
                                                                           (pgm_read_byte(&attributes[i].readonly) & READONLY));
                        // data in RAM
                        *p_attr_ref = {
                            section,
                            option,
                            (uint8_t) (offset + attr_offset),
                            (uint8_t) MIN(attr_size - attr_offset, 4),
                        };

#if LOG_LEVEL_DBG
                        print_attr_ref(p_attr_ref);
#endif

                        return CANIOT_OK;
                    }
                    else
                    {
                        return CANIOT_EKEYPART;
                    }
                }
                else
                {
                    offset += attr_size;
                    attr_index++;
                }
            }
        }
        return CANIOT_EKEYATTR;
    }
    return CANIOT_EKEYSECTION;
}

// todo shorten this switch with an array of pointers, get rid of nullptr check
void *can_device::get_section_address(const uint8_t section)
{
    switch (section)
    {
    case ATTR_IDENTIFICATION:
        return p_instance->p_identification;

    case ATTR_SYSTEM:
        return p_instance->p_system;

    case ATTR_CONFIG:
        return p_instance->p_config;

    case ATTR_SCHEDULES:
        return p_instance->p_schedules;

    default:
        return nullptr;
    }
}

const uint8_t can_device::read_attribute(const attr_ref_t *const attr_ref, value_t *const p_value)
{
    uint8_t err = CANIOT_NULL;
    if (attr_ref != nullptr)
    {
        const void *p = (void *)((uint16_t)get_section_address(attr_ref->section) + attr_ref->offset);
        if (attr_ref->options & RAM) // priority for RAM
        {
            memcpy(p_value, p, attr_ref->read_size);
        }
        else if (attr_ref->options & PROGMEMORY)
        {
            memcpy_P(p_value, p, attr_ref->read_size);
        }
        else if (attr_ref->options & EEPROM)
        {
            eeprom_read_block(p_value, p, attr_ref->read_size);
        }
        else
        {
            return CANIOT_ENIMPL;
        }

        return CANIOT_OK;
    }
    return err;
}

const uint8_t can_device::write_attribute(const attr_ref_t *const attr_ref, const value_t value)
{
    uint8_t err = CANIOT_NULL;
    if (attr_ref != nullptr)
    {
        if (attr_ref->options & READONLY)
        {
            return CANIOT_EREADONLY;
        }

        void* p = (void *)((uint16_t)get_section_address(attr_ref->section) + attr_ref->offset);
        if (attr_ref->options & RAM)
        {
            memcpy(p, (void *)&value, attr_ref->read_size);
        }
        else if (attr_ref->options & PROGMEMORY)
        {
            return CANIOT_EREADONLY;
        }
        else if (attr_ref->options & EEPROM)
        {
            eeprom_write_block(p, (void *)&value, attr_ref->read_size);

            // warning if many writings
        }
        else
        {
            return CANIOT_ENIMPL;
        }

        return CANIOT_OK;
    }
    return err;
}

/*___________________________________________________________________________*/

void can_device::print_identification(void)
{
    usart_print("name    = ");
    usart_printl(p_identification->name);

    usart_print("id      = ");
    usart_hex(p_identification->device.id);
    usart_transmit('\n');

    usart_print("type    = ");
    usart_hex(p_identification->device.type);
    usart_transmit('\t');

    print_prog_data_type((data_type_t) p_identification->device.type);

    usart_print("\nversion = ");
    usart_u16(p_identification->version);
    usart_transmit('\n');
}

/*___________________________________________________________________________*/