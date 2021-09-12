#include "scheduler.h"

#include "device.h"

/*___________________________________________________________________________*/

DEFINE_TQUEUE(events_queue);

/*___________________________________________________________________________*/

void schedule(event_t& event, timeout_t timeout)
{
    cli();
    if (!event.scheduled) {
        event.scheduled = true;
        tqueue_schedule(&events_queue, &event.tie, timeout);
    }
    sei();
}

void unschedule(event_t& event)
{
    cli();
    if (event.scheduled) {
        event.scheduled = false;
        tqueue_remove(&events_queue, &event.tie);
    }
    sei();
}

uint8_t scheduler_process(void)
{
    cli();
    struct titem* item = tqueue_pop(&events_queue);
    sei();

    if (item != nullptr) {
        event_t* p_event = CONTAINER_OF(item, event_t, tie);
        p_event->scheduled = false;
        if (p_event->handler != nullptr) {
            /* don't alter event related data until the handler is executed */
            return p_event->handler((void*)p_event);    
        } else {
            return CANIOT_EENOCB;
        }
    }
    return CANIOT_OK;
}

/*___________________________________________________________________________*/