#ifndef _CANIOT_UTILS_
#define _CANIOT_UTILS_

#include <stdint.h>
#include <stddef.h>

#include <avr/pgmspace.h>

#include "uart.h"
#include "device.h"

/*___________________________________________________________________________*/

uint8_t xor_checksum(void* const data, const size_t size);

/*___________________________________________________________________________*/

#endif