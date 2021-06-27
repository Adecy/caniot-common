#ifndef _TIMER2_H
#define _TIMER2_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/atomic.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t time_cs_t;
typedef uint32_t time_s_t;

void timer2_init(void);

void timer2_copy_counter(uint32_t *p_copy);

// uptime in second
void timer2_uptime(time_s_t *p_uptime);

// uptime in centi second (10^-2 second)
void timer2_uptime_centiseconds(time_cs_t *p_uptime_cs);

#ifdef __cplusplus
}
#endif

#endif