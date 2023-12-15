#ifndef HEADERS_H
#define HEADERS_H

#include <stdio.h> //if you don't use scanf/printf change this include
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>

typedef short bool;
#define true 1
#define false 0

#define SHKEY 300
#define MSG_QUEUE_SHCEDULAR_PROCESS_KEY 107
#define MSG_QUEUE_GENERATOR_SCHEDULAR_KEY 503
#define KEY_FILE "keyfile"
//#define MAX_NUM_OF_PROCESS 5
#define PROCESS_EXECUTABLE_NAME "process.out"
#define MAX_NUM_OF_RESOURCES 5

int MAX_NUM_OF_PROCESS;

typedef struct buffer_item
{
    long mtype;
    int mIntegerData;
} messageBuff;

typedef enum
{
    // RUN,
    // BLOCKED,
    // READY
    STARTED,
    RESUMED,
    STOPPED,
    FINISHED
} State;

typedef enum SchedulingAlgorithms
{
    HPF,
    SRTN,
    RR
} SchedulingAlgorithm;

typedef struct Process
{
    long mtype;
    int id;
    int arrivalTime;
    int runTime;
    int priority;
} Process;

typedef int ProcessID;
typedef int Time;
typedef u_int16_t Priority;

typedef struct
{
    State state;
    ProcessID processID;       // process ID recieved from process generator
    ProcessID mappedProcessID; // actual process ID in the os
    Time runTime;              
    Time arrivalTime;
    Time startTime;
    Time remainingTime;
    Time finishTime;
    Priority priority;
    size_t pqPosition;
    Time wait; 
    Time TA;   
    float WTA;  

} PCB;

///==============================
// don't mess with this variable//
int *shmaddr; //
//===============================

int getClk()
{
    return *shmaddr;
}

/*
 * All process call this function at the beginning to establish communication between them and the clock module.
 * Again, remember that the clock is only emulation!
 */
void initClk()
{
    int shmid = shmget(SHKEY, 4, 0444);
    while ((int)shmid == -1)
    {
        // Make sure that the clock exists
        printf("Wait! The clock not initialized yet!\n");
        //sleep(1);
        shmid = shmget(SHKEY, 4, 0444);
    }
    shmaddr = (int *)shmat(shmid, (void *)0, 0);
}

/*
 * All process call this function at the end to release the communication
 * resources between them and the clock module.
 * Again, Remember that the clock is only emulation!
 * Input: terminateAll: a flag to indicate whether that this is the end of simulation.
 *                      It terminates the whole system and releases resources.
 */

void destroyClk(bool terminateAll)
{
    shmdt(shmaddr);
    if (terminateAll)
    {
        killpg(getpgrp(), SIGINT);
    }
}

#endif