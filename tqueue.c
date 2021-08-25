#include "tqueue.h"

// A should be processed in 10 ms
// B should be processed in 10 + 30 = 40ms
// C should be processed in 40 + 0 = 40ms
// D should be processed in 40 + 10 = 50ms
// 0 |-- A(10) -- B(30) -- C(0) -- D(10)

// adding E which should be processed in 25ms

// we need to insert i beetween A and B in order to have :
// 0 |-- A(10) -- E(15) -- B(15) -- C(0) -- D(10)


/*___________________________________________________________________________*/

void _tqueue_schedule(struct titem **root, struct titem *item)
{
    struct titem ** prev_next_p = root;
    while (*prev_next_p != NULL)
    {
        struct titem * p_current = *prev_next_p; // next of previous become current

        // if current element expires before or at the same time that item delay, we go to next item
        if (p_current->delay_shift <= item->delay_shift)
        {
            item->delay_shift -= p_current->delay_shift;
            prev_next_p = &(p_current->next);
        }
        else    // if current element expire after, we need to insert the new_item before current and linked *prev_next_p
        {
            item->next = p_current;
            p_current->delay_shift -= item->delay_shift;
            break;
        }
    }
    *prev_next_p = item;
}

void tqueue_schedule(struct titem **root, struct titem *item, k_delta_ms_t timeout)
{
    if (item == NULL)
        return;

    item->next = NULL;  // necessary if the item is put at the very end of the list
    item->timeout = timeout;

    _tqueue_schedule(root, item);
}

void tqueue_shift(struct titem **root, k_delta_ms_t time_passed)
{
    struct titem ** prev_next_p = root;
    while (*prev_next_p != NULL) // if next of previous item is set
    {
        struct titem * p_current = *prev_next_p; // next of previous become current
        if (p_current->delay_shift <= time_passed)
        {
            if (p_current->delay_shift != 0)    // if item delay is already 0 
            {
                time_passed -= p_current->delay_shift;
                p_current->delay_shift = 0;
            }
            prev_next_p = &(p_current->next);
        }
        else
        {
            p_current->delay_shift -= time_passed;
            break;
        }
    }
}

struct titem * tqueue_pop(struct titem **root)
{
    struct titem * item = NULL;
    if ((*root != NULL) && ((*root)->delay_shift == 0)) // if next to expire has expired
    {
        item = *root;    // prepare to return it
        *root = (*root)->next;
    }
    return item;
}

void tqueue_remove(struct titem **root, struct titem *item)
{
    struct titem **prev_next_p = root;
    while (*prev_next_p != NULL)
    {
        struct titem * p_current = *prev_next_p;
        if (p_current == item)
        {
            *prev_next_p = p_current->next;
            if (p_current->next != NULL)
            {
                p_current->next->delay_shift += item->delay_shift;    // add removed item remaining time to the next item if exists
            }
            
            item->next = NULL;
            break;
        }
        prev_next_p = &(p_current->next);
    }
}

/*___________________________________________________________________________*/