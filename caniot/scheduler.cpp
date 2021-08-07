#include "scheduler.h"

#include "device.h"

/*___________________________________________________________________________*/

DEFINE_TQUEUE(events_queue);

/*___________________________________________________________________________*/

void schedule(event_t &event, timeout_t timeout)
{
    cli();
    tqueue_schedule(&events_queue, &event.tie, timeout);
    sei();
}

void unschedule(event_t &event)
{
    cli();
    tqueue_remove(&events_queue, &event.tie);
    sei();
}

uint8_t scheduler_process(void)
{
    cli();
    struct titem* item = tqueue_pop(&events_queue);
    sei();

    if (item != nullptr)
    {
        item->next = nullptr;
        event_t * p_event = CONTAINER_OF(item, event_t, tie);
        if (p_event->handler != nullptr)
        {
            return p_event->handler((void*) p_event);
        }
        else
        {
            return CANIOT_EENOCB;
        }
    }
    return CANIOT_OK;
}

/*___________________________________________________________________________*/