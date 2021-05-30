#ifndef _CANIOT_HW_INIT_H
#define _CANIOT_HW_INIT_H

#include "caniot.h"

#define ARDUINO_ENABLE_ANALOG   0
#define ARDUINO_ENABLE_FAST_PWM 0

#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))


void hw_init(void);

#endif