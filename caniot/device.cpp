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
    if (p_config->telemetry_period && (uptime() - p_system->last_telemetry >= p_config->telemetry_period))
    {
        SET_FLAG_TELEMETRY(flags);
    }

    if (TEST_FLAG_COMMAND(flags))
    {
        process_query();
    }

    if (TEST_FLAG_TELEMETRY(flags))
    {
        process_telemetry();
    }
}

void can_device::process_query(void)
{
    uint8_t err = CAN_OK;

    if (CAN_MSGAVAIL == p_can->checkReceive())
    {
        p_can->readMsgBufID(&request.id.value, &request.len, request.buffer);

        print_can_expl(request);

        memset(response.buffer, 0x00, 8);
        err = dispatch_request(request, response);
        if (request.need_response())
        {
            if (err == CANIOT_OK)
            {
                print_can_expl(response);

                err = send_response(response);
            }
            else if (LOOPBACK_IF_ERR)
            {
                err = send_response(request);
            }
        }
    }
    else
    {
        CLEAR_FLAG_COMMAND(flags);
    }

    // handle errors
    if (err != CANIOT_OK)
    {
        usart_print("Error command handling : ");
        usart_hex(err);
        usart_transmit('\n');
    }
}

void can_device::process_telemetry(void)
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
                response.id.value = BUILD_ID(type_t::telemetry, query_t::response, controller_t::broadcast, __DEVICE_TYPE__, __DEVICE_ID__);

                print_can_expl(response);
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

    // handle errors
    if (err != CANIOT_OK)
    {
        usart_print("Error telemetry : ");
        usart_hex(err);
        usart_transmit('\n');
    }
}

uint8_t can_device::dispatch_request(Message &request, Message &response)
{
    uint8_t ret = CANIOT_ERROR;

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
            if (request.len == 2)
            {
                ret = read_attribute(*(key_t *) request.buffer, (value_t*) response.buffer);
            }
            break;

        case type_t::write_attribute:
            if ((request.len >= 3) && (request.len <= 6))
            {
                ret = write_attribute(*(key_t *)request.buffer, *(value_t *)&request.buffer[2]);
            }
            break;

        case type_t::telemetry:
            request_telemetry();
            ret = CANIOT_OK;
            break;
            
        default:
            ret = CANIOT_ENPROC; // error unprocessable request type
        }

        if (ret == CANIOT_OK)   // success
        {
            p_system->message_received++;
        }
        else    // error
        {
            response.set_errno(ret);
        }

        return ret;
    }
    else
    {
        // entity not processable
        return CANIOT_ERROR;
    }
}


uint8_t can_device::send_response(Message &response)
{
    return p_can->sendMsgBuf(response.id.value, CAN_STDID, response.len, response.buffer);
}

void can_device::request_telemetry(void)
{
    SET_FLAG_TELEMETRY(flags);
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
                                                                           (pgm_read_byte(&attributes[i].readonly) & (1 << 3)));
                        *p_attr_ref = {
                            section,
                            option,
                            (uint8_t)(offset + attr_offset),
                            (uint8_t)(attr_size - attr_offset)};

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

const uint8_t can_device::read_attribute(const key_t key, value_t *const p_value)
{
    attr_ref_t attr_ref;

    const uint8_t err = resolve_attribute(key, &attr_ref);
    if (err == CANIOT_OK)
    {
        if (attr_ref.options & READONLY)
        {
            return CANIOT_EREADONLY;
        }

        const void *p = get_section_address(attr_ref.section);
        
        if ((p != nullptr) || (attr_ref.options & EEPROM)) // nullptr is a valid EEPROM address
        {
            p = (void *)((uint16_t)p + attr_ref.offset);

            if (attr_ref.options & RAM)
            {
                memcpy(p_value, p, attr_ref.size);
            }
            else if (attr_ref.options & PROGMEMORY)
            {
                memcpy_P(p_value, p, attr_ref.size);
            }
            else if (attr_ref.options & EEPROM)
            {
                eeprom_read_block(p_value, p, attr_ref.size);
            }
            else
            {
                return CANIOT_ENIMPL;
            }

            return CANIOT_OK;
        }
        else
        {
            return CANIOT_ENIMPL;
        }
    }
    return err;
}

const uint8_t can_device::write_attribute(const key_t key, const value_t value)
{
    attr_ref_t attr_ref;

    const uint8_t err = resolve_attribute(key, &attr_ref);
    if (err == CANIOT_OK)
    {
        void *p = get_section_address(attr_ref.section);
        if (p != nullptr)
        {
            p = (void *)((uint16_t)p + attr_ref.offset);

            if (attr_ref.options & RAM)
            {
                memcpy(p, (void *)&value, attr_ref.size);
            }
            else if (attr_ref.options & PROGMEMORY)
            {
                return CANIOT_EREADONLY;
            }
            else if (attr_ref.options & EEPROM)
            {
                eeprom_write_block(p, (void *)&value, attr_ref.size);
            }
            else
            {
                return CANIOT_ENIMPL;
            }

            return CANIOT_OK;
        }
        else
        {
            return CANIOT_ENIMPL;
        }
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