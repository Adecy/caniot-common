#include "device.h"

#include <errno.h>

can_device *can_device::p_instance;

extern can_device::identification_t can_device::identification;

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

        m_error = CANIOT_ERR_DRIVER;
    };

    m_error = CANIOT_ERR_OK;

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
}



void can_device::process(void)
{
    while (CAN_MSGAVAIL == p_can->checkReceive())
    {
        p_can->readMsgBufID(&request.id.value, &request.len, request.buffer);

        print_can_expl(request.id, request.buffer, request.len);

        int ret = dispatch_request(request, response);
        if (ret == 0)
        {
            p_can->sendMsgBuf(response.id.value, CAN_STDID, response.len, response.buffer);
        }
    }
}

int can_device::dispatch_request(Message &request, Message &response)
{
    int ret;

    if (request.is_query())
    {
        switch (request.get_type())
        {
            case type_t::command:
                // todo
                break;

            case type_t::read_attribute:
                ret = handle_read_attribute(*(uint16_t*) request.buffer, response);
                break;

            case type_t::write_attribute:
                ret = handle_write_attribute(*(uint16_t*) request.buffer, *(uint32_t*) &request.buffer[2], response);
                break;

            case type_t::telemetry:
            default:
                return -1;  // error unprocessable request type
        }

        return ret;
    }
    else
    {
        // entity not processable
        return -1;
    }

    return 0;
}

int can_device::handle_read_attribute(uint16_t key, Message &response)
{
    
    return 0;
}

int can_device::handle_write_attribute(uint16_t key, uint32_t value, Message &response)
{
    return 0;
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
    usart_transmit('\n');

    usart_print("version = ");
    usart_u16(identification.version);
    usart_transmit('\n');
}

/*___________________________________________________________________________*/