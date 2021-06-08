#include "device.h"

#include <errno.h>

can_device *can_device::p_instance;

extern struct can_device::attributes can_device::attributes;

/*___________________________________________________________________________*/

ISR(INT0_vect)
{
    can_device::get_instance()->flag_can = 1u;
}

void can_device::initialize(void)
{
    while (CAN_OK != p_can->begin(m_speedset, m_clockset))
    {
        _delay_ms(1000);

        m_error = CANIOT_EDRIVER;
    };

    m_error = CANIOT_OK;

    p_can->init_Mask(0, CAN_STDID, DEVICE_RXM0);
    p_can->init_Filt(0, CAN_STDID, DEVICE_RXF0);
    p_can->init_Filt(1, CAN_STDID, DEVICE_RXF1);

    p_can->init_Mask(1, CAN_STDID, DEVICE_RXM1);
    p_can->init_Filt(2, CAN_STDID, DEVICE_RXF2);
    p_can->init_Filt(3, CAN_STDID, DEVICE_RXF3);
    p_can->init_Filt(4, CAN_STDID, DEVICE_RXF4);
    p_can->init_Filt(5, CAN_STDID, DEVICE_RXF5);

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

    if (flag_can)
    {
        while (CAN_MSGAVAIL == p_can->checkReceive())
        {
            p_can->readMsgBufID(&request.id.value, &request.len, request.buffer);

            print_can_expl(request);

            int ret = dispatch_request(request, response);
            if (ret == CANIOT_OK)
            {
                print_can_expl(response);
                send_response(response);
            }
            else if (LOOPBACK_IF_ERR)
            {
                send_response(request);
            }
        }
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
                ret = handle_command(request.get_data_type(), response);
                break;

            case type_t::read_attribute:
                if (request.len == 2)
                {
                    ret = read_attribute(*(uint16_t*) request.buffer, response);
                }
                break;

            case type_t::write_attribute:
                if ((request.len >= 3) && (request.len <= 6))
                {
                    ret = write_attribute(*(uint16_t*) request.buffer, *(uint32_t*) &request.buffer[2], response);
                }
                break;

            case type_t::telemetry:
            default:
                ret = CANIOT_ENPROC;  // error unprocessable request type
        }

        if (ret == CANIOT_OK)   // success
        {
            p_system->message_received++;
        }
        else    // error
        {
            response.set_errno(-ret);
        }

        return CANIOT_OK;
    }
    else
    {
        // entity not processable
        return CANIOT_ERROR;
    }
}

uint8_t can_device::read_attribute(const uint16_t key, Message &response)
{
    if (key == 0x0101)
    {
        *(uint32_t*) response.buffer = p_system->abstime;
        response.len = 4u;

        usart_print("read_attribute abstime");
    }

    return 0;
}

uint8_t can_device::write_attribute(const uint16_t key, const uint32_t value, Message &response)
{
    if (key == 0x0101)  // set abstime
    {
        p_system->abstime = value;
        p_system->uptime_shift = p_system->uptime;
    }

    return read_attribute(key, response);
}

uint8_t can_device::handle_command(const data_type_t data_type, Message &response)
{
    return 0;
}

uint8_t can_device::send_response(Message &response)
{
    return p_can->sendMsgBuf(response.id.value, CAN_STDID, response.len, response.buffer);
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
    usart_transmit('\n');

    usart_print("version = ");
    usart_u16(p_identification->version);
    usart_transmit('\n');
}

/*___________________________________________________________________________*/