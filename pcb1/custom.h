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

#define IN0     0
#define IN1     1
#define IN2     2
#define IN3     3

#define IN0_PORT     PIND3
#define IN1_PORT     PIND4
#define IN2_PORT     PIND5
#define IN3_PORT     PIND6

union inputs
{
    struct {
        uint8_t in0 : 1;
        uint8_t in1 : 1;
        uint8_t in2 : 1;
        uint8_t in3 : 1;
    };
    uint8_t ins;
};

/*___________________________________________________________________________*/

class CustomBoard : public can_device
{
protected:
    mcp2515_can can = mcp2515_can(SPI_CS_PIN);

public:
    CustomBoard(uint32_t speedset, uint8_t clockset) :
        can_device(&can, CAN_INT_PIN, CAN_SPEED, MCP_16MHz) { }

    void initialize(void);
    
    static uint8_t read_inputs(void);
    static int16_t read_temperature(void);

protected:
    void io_init(void);
};

/*___________________________________________________________________________*/

#endif