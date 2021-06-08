#ifndef _TIMER2_H
#define _TIMER2_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

void timer2_init(void);

void timer2_copy_counter(uint32_t *p_copy);

// uptime in second
void timer2_uptime(uint32_t *p_uptime);

#ifdef __cplusplus
}
#endif

#endif