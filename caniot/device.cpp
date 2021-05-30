#include "device.h"

can_device *can_device::p_instance;

const can_device::identification_t can_device::identification PROGMEM = {
    __DEVICE_NAME__,
    __DEVICE_ID__,
    __DEVICE_TYPE__,
    __FIRMWARE_VERSION__
};

/*___________________________________________________________________________*/

ISR(INT0_vect)
{
    can_device::get_instance()->flag_can = 1u;
}

void can_device::initialize(void)
{
    while (CAN_OK != can.begin(m_speedset, m_clockset))
    {
        _delay_ms(1000);

        m_error = CANIOT_ERR_DRIVER;
    };

    m_error = CANIOT_ERR_OK;

    // interrupt when receiving a can message
    // falling on INT0
    EICRA |= 1 << ISC01;
    EICRA &= ~(1 << ISC00);
    EIMSK |= 1 << INTF0;
}

void can_device::process_message(void)
{
    if (CAN_MSGAVAIL == p_can->checkReceive())
    {
        p_can->readMsgBufID(&id.value, &len, buffer);

        // print_can(id.value, buffer, len);

        print_can_expl(id, buffer, len);

        // loopback
        p_can->sendMsgBuf(id.value, CAN_STDID, len, buffer);
    }
}

/*___________________________________________________________________________*/

void can_device::print_identification(void)
{
    uint8_t id;
    uint8_t type;
    uint16_t version;

    memcpy_P(&id, &identification.id, 1);
    memcpy_P(&type, &identification.type, 1);
    memcpy_P(&version, &identification.version, 2);

    usart_print("name    = ");
    usart_printl_p(identification.name);

    usart_print("id      = ");
    usart_hex(id);
    usart_transmit('\n');

    usart_print("type    = ");
    usart_hex(type);
    usart_transmit('\n');

    usart_print("version = ");
    usart_u16(version);
    usart_transmit('\n');
}