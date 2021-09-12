#include "timer2.h"
#include "scheduler.h"
#include "tqueue.h"

volatile uint32_t timer2_counter = 0;

// 100Hz timer
ISR(TIMER2_OVF_vect)
{
    TCNT2 = 256 - 156;
    
    tqueue_shift(&events_queue, TIMER2_PERIOD);

    timer2_counter++;
}

void timer2_init(void)
{
    // normal mode
    TCCR2A &= ~(1 << WGM20 | 1 << WGM21 | 1 << WGM22);

    // set init TCNT2
    // prescaler = 1024
    TCCR2B |= (1 << CS20 | 1 << CS21 | 1 << CS22);

    // set counter
    TCNT2 = 256 - 156;

    // enable interrupt mask, start timer
    TIMSK2 |= 1 << TOIE2;
}

void timer2_copy_counter(uint32_t *p_copy)
{
    cli();
    *p_copy = timer2_counter;
    sei();
}

void timer2_uptime_centiseconds(time_cs_t *p_uptime_cs)
{
    timer2_copy_counter(p_uptime_cs);
}

void timer2_uptime(time_s_t *p_uptime)
{
    uint32_t uptime_cs;
    timer2_uptime_centiseconds(&uptime_cs);
    *p_uptime = uptime_cs / 100;
}