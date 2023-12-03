#include "headers.h"
typedef u_int16_t ProcessID;
typedef u_int16_t Time;
typedef u_int16_t Priority;

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
    ProcessID mappedProcessID;
    Time arrivalTime;
    Time startTime;
    Time remainingTime;
    Time finishTime;
    Priority priority;

} PCB;

static void *createPCBEntry(Process *newProcess)
{
    PCB *newPCBEntry = (PCB *)malloc(sizeof(PCB));
    newPCBEntry->arrivalTime = newProcess->arrivalTime;
    newPCBEntry->remainingTime = newProcess->runTime;
    newPCBEntry->priority = newProcess->priority;
    newPCBEntry->processID = newProcess->id;
    return newPCBEntry;
}

static void deletePCBEnrty(PCB *pcbEntry)
{
    free(pcbEntry);
}

static void addToRR(PCB *pcbEntry)
{
}

static void addToHPF(PCB *pcbEntry)
{
}

static void addToSRTN(PCB *pcbEntry)
{
}

static receiveProcesses()
{
}

int main(int argc, char *argv[])
{
    initClk();

    // TODO implement the scheduler :)
    // upon termination release the clock resources.

    destroyClk(true);
}
