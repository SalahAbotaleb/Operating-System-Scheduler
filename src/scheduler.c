#include "headers.h"
typedef u_int32_t ProcessID;
typedef u_int32_t Time;

typedef enum
{
    RUN,
    BLOCKED,
    READY
} State;

typedef struct
{
    State state;
    ProcessID processID;
    Time startTime;
    Time remainingTime;
    Time finishTime;

} PCB;

static void *createPCBEntry(Process *newProcess)
{
}

int main(int argc, char *argv[])
{
    initClk();

    // TODO implement the scheduler :)
    // upon termination release the clock resources.

    destroyClk(true);
}
