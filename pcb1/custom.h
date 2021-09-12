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
#define CAN_INT_PIN                 2

#define CAN_SPEED                   CAN_500KBPS

/*___________________________________________________________________________*/

class CustomBoard : public can_device
{
protected:
    mcp2515_can can = mcp2515_can(SPI_CS_PIN);

public:
    CustomBoard(uint32_t speedset, uint8_t clockset) :
        can_device(&can, CAN_INT_PIN, CAN_SPEED, MCP_16MHz) { }

    void initialize(void);

    int16_t read_temperature(void);

protected:
    void io_init(void);
};

/*___________________________________________________________________________*/

#endif