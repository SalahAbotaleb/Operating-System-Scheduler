#include "Data_Structures/pqueue.h"
#include "headers.h"
/**Priority Queue Callback functions**/
static int
cmpPriority(pqueue_pri_t next, pqueue_pri_t curr)
{
    return (next > curr);
}

static pqueue_pri_t
getPriority(void *a)
{
    return ((PCB *)a)->priority;
}

static pqueue_pri_t
getRemainingTimeAsPriority(void *a)
{
    return ((PCB *)a)->remainingTime;
}

static void
setPriority(void *a, pqueue_pri_t pri)
{
    ((PCB *)a)->priority = pri;
}

static void
setRemainingTimeAsPriority(void *a, pqueue_pri_t pri)
{
    ((PCB *)a)->remainingTime = pri;
}

static size_t
get_pos(void *a)
{
    return ((PCB *)a)->pqPosition;
}

static void
set_pos(void *a, size_t pos)
{
    ((PCB *)a)->pqPosition = pos;
}
/**End of Priority Queue Callback functions**/