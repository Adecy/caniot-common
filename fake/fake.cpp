#include "fake.h"

/*___________________________________________________________________________*/


void FakeDevice::initialize(void)
{
    hw_init();
    usart_init();

    SMCR = SLEEP_MODE_PWR_SAVE; // this is an atomic write

    can_device::initialize();    

    memset(buffer_val, 0x00, sizeof(buffer_val));

    set_command_handler(command_handler);
    set_telemetry_builder(telemetry_builder);
}

uint8_t FakeDevice::command_handler(uint8_t buffer[8], uint8_t len)
{
    FakeDevice * fake = (FakeDevice*) get_instance();

    memset(fake->buffer_val + 8u - len, 0x00, 8u - len);
    memcpy(fake->buffer_val, buffer, len);
    
    return CANIOT_OK;
}

uint8_t FakeDevice::telemetry_builder(uint8_t buffer[8], uint8_t &len)
{
    FakeDevice * fake = (FakeDevice*) get_instance();

    len = CANIOT_GET_LEN(U);
    memcpy(buffer, fake->buffer_val, len);    

    return CANIOT_OK;
}