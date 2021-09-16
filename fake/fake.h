#ifndef _CUSTOM_BOARD_H
#define _CUSTOM_BOARD_H

#include <avr/io.h>
#include <stddef.h>
#include <stdint.h>

#include <caniot/device.h>
#include <caniot/hw_init.h>

#include <mcp_can.h>
#include <Wire.h>

/*___________________________________________________________________________*/

#define SPI_CS_PIN                  10

/*___________________________________________________________________________*/

class FakeDevice : public can_device
{
protected:
    mcp2515_can can = mcp2515_can(SPI_CS_PIN);

    uint8_t buffer_val[8];

public:
    FakeDevice(uint8_t ext_int_pin, uint32_t speedset, uint8_t clockset) :
        can_device(&can, ext_int_pin, speedset, clockset) { }

    void initialize(void);
    
    static uint8_t command_handler(uint8_t buffer[8], uint8_t len);
    static uint8_t telemetry_builder(uint8_t buffer[8], uint8_t &len);
};

/*___________________________________________________________________________*/

#endif