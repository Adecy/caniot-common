#ifndef _CANIOT_UTILS_
#define _CANIOT_UTILS_

#include <stdint.h>

#include <avr/pgmspace.h>

#include "uart.h"
#include "device.h"

void print_can(const unsigned long id, const uint8_t * const buffer, const uint8_t len);

void print_prog_string(PGM_P const pgm_string_array, const uint8_t elem);

void print_can_expl(can_id_t id, const uint8_t * const buffer, const uint8_t len);

void debug_sizeof(void);


#endif