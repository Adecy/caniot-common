#ifndef _AVRTOS_SCHEDULER
#define _AVRTOS_SCHEDULER

#include <stdbool.h>

#include "tqueue.h"

/*___________________________________________________________________________*/

struct event_t;

typedef uint8_t(*event_handler_t)(struct event_t* ev);

struct event_t
{
    event_handler_t handler;
    struct titem tie;
    bool scheduled;
};

extern struct titem* events_queue;

#define EVENT_DEFINE(evname, p_handler)            \
    struct event_t evname = {                              \
        .handler = p_handler,                              \
        .tie = INIT_TITEM(0),                              \
        .scheduled = false,                                \
    }

/*___________________________________________________________________________*/

#ifdef __cplusplus

void schedule(event_t& event, timeout_t timeout);

void unschedule(event_t& event);

uint8_t scheduler_process(void);

#endif

/*___________________________________________________________________________*/

#endif