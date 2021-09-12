#ifndef _AVRTOS_SCHEDULER
#define _AVRTOS_SCHEDULER

#include "tqueue.h"

/*___________________________________________________________________________*/

typedef uint8_t(*event_handler_t)(void*);
typedef struct
{
    event_handler_t handler;
    struct titem tie;
} event_t;

extern struct titem* events_queue;

#define EVENT_DEFINE_CONTEXT(evname, p_handler) \
    event_t evname = {                                     \
        .handler = p_handler,                              \
        .tie = {                                           \
            {.timeout = 0},                                \
            .next = nullptr,                               \
        }                                                  \
    }

/*___________________________________________________________________________*/

#ifdef __cplusplus

void schedule(event_t& event, timeout_t timeout);

void unschedule(event_t& event);

uint8_t scheduler_process(void);

#endif

/*___________________________________________________________________________*/

#endif