#ifndef _CANIOT_UTILS_
#define _CANIOT_UTILS_

#include <stdint.h>

#include <avr/pgmspace.h>

#include "uart.h"
#include "device.h"

/*___________________________________________________________________________*/

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))

/*___________________________________________________________________________*/

class Message;

/*___________________________________________________________________________*/

void print_can(const unsigned long id, const uint8_t * const buffer, const uint8_t len);

void print_prog_string(PGM_P const pgm_string_array, const uint8_t elem);

void print_prog_query(const query_t qr);
void print_prog_data_type(const data_type_t dt);
void print_prog_controller(const controller_t ctrl);

void print_can_expl(Message &can_msg);

void print_can_expl(can_id_t id, const uint8_t * const buffer, const uint8_t len);

void print_time_sec(uint32_t time_sec);

/*___________________________________________________________________________*/

void debug_masks_filters(void);

/*___________________________________________________________________________*/


#endif