#include "headers.h"
#include "Data_Structures/linked_list.h"
#include "Data_Structures/pqueue.h"
#include "pqCustomizedCB.c"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>

typedef void (*algorithm)(void *);
typedef void (*addItem)(void *, PCB *);
typedef void *(*createDS)();

static int lastclk = 0;
static Node *lastNode = NULL; // it is used in processRR & processSRTN
static PCB **processTable = NULL;
static SchedulingAlgorithm algorithmType;
static pqueue_t *pqHPF = NULL;  // priority queue for Highest Priority First Algorithm
static pqueue_t *pqSRTN = NULL; // priority queue for Shortet Remaining Time Next Algorithm
static LinkedList *llRR = NULL; // linked list for Round Robin Algorithm
static int resources[MAX_NUM_OF_RESOURCES] = {-1};
static int currentResourcesCount = 0;
static int lstProcessKilled = -2;
static PCB *lstPCB = NULL;

void stopProcess(PCB *process)
{
    kill(process->mappedProcessID, SIGSTOP);
}

void contiuneProcess(PCB *process)
{
    kill(process->mappedProcessID, SIGCONT);
}

static void addTest(PCB *process)
{
    printf("Date: %d, %d, %d, %d\n", process->arrivalTime, process->remainingTime,
           process->priority, process->processID);
}

static PCB *createPCBEntry(Process *newProcess)
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

static void *createLL()
{
    return CreateLinkedList();
}

static void addToRR(void *listT, PCB *pEntry)
{
    LinkedList *list = (LinkedList *)listT;
    AddNodeToBack(list, CreateNode(pEntry));
}

static void addToHPF(void *pqT, PCB *pEntry)
{
    pqueue_t *pq = (pqueue_t *)pqT;
    pqueue_insert(pq, pEntry);
}

static void addToSRTN(void *pqT, PCB *pEntry)
{
    pqueue_t *pq = (pqueue_t *)pqT;
    pqueue_insert(pq, pEntry);
}

static void processRR(void *listT)
{
    LinkedList *list = (LinkedList *)listT;

    int clock = getClk();
    if (clock != lastclk)
    {
        lastclk = clock;
        if (list->size > 0)
        {
            if (lastNode == NULL)
            {
                lastNode = list->head;
            }
            else
            {
                // TODO stop current process
                if (lastNode->nxt == NULL)
                {
                    lastNode = list->head;
                }
                else
                {
                    lastNode = lastNode->nxt;
                }
            }
            PCB *process = lastNode->data;
            /**
             * Note: read process it self
             * it is the one reponsible for sending that is has finished
             * There is no problem that we keep track of remaining time
             */
            process->remainingTime--;
            if (process->remainingTime == 0)
            {
                // TODO send message to process generator
                // TODO delete node
            }
            else
            {
                // TODO start process
            }
        }
    }
}

static void processHPF(void *pqT)
{
    pqueue_t *pq = pqT;
    if (pq->size <= 1)
        return;

    static ProcessID currProcess = -1;
    PCB *highestPriorityProcess = pqueue_peek(pq);
    if (highestPriorityProcess == NULL)
    {
        return;
    }
    if (currProcess == -1)
    {
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        contiuneProcess(lstPCB);
    }
    else if (lstProcessKilled == currProcess)
    {
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        contiuneProcess(lstPCB);
    }
}

static void processSRTN(void *pqT)
{
    pqueue_t *pq = pqT;
    // should continue function here
    // TODO: copy commented part and pase here
}

// static void processSRTN(pqueue_t *pq)
// {
//     int clock = getClk();
//     if (clock != lastclk)
//     {
//         lastclk = clock;
//         if (pq->size > 0)
//         {
//             if (lastNode == NULL)
//             {
//                 lastNode = pq->head;
//                 lastNode->data->state = RUN;
//                 pid_t pid = fork();
//                 if (pid == 0)
//                 {
//                     char *args[] = {"./process.out", NULL};
//                     execvp(args[0], args);
//                 }
//                 else
//                 {
//                     lastNode->data->startTime = getClk();
//                     lastNode->data->mappedProcessID = getpid(); //@uncertain what do you mean by mappedProcessID
//                     lastNode->data->pid = getpid();
//                 }
//             }

//             PCB *lastProcess = lastNode->data;
//             /**
//              * Note: read process it self
//              * it is the one reponsible for sending that is has finished
//              * There is no problem that we keep track of remaining time
//              */

//             if (lastProcess->remainingTime == 0)
//             {
//                 // TODO send message to process generator
//                 // TODO delete node
//                 lastProcess->finishTime = getClk();
//                 PCB *finishedPCB = pop(pq);
//                 // deletePCBEnrty(finishedPCB);
//                 lastProcess = pq->head->data; //@uncertain will pop heapfy priority queue
//                 lastProcess->state = RUN;
//                 pid_t pid = fork();
//                 if (pid == 0)
//                 {
//                     char *args[] = {"./process.out", NULL};
//                     execvp(args[0], args);
//                 }
//                 else
//                 {
//                     lastProcess->startTime = getClk();
//                     lastProcess->mappedProcessID = getpid(); //@uncertain what do you mean by mappedProcessID
//                     lastProcess->pid = getpid();
//                 }
//             }
//             else
//             {
//                 if (lastProcess != pq->head->data) //@uncertain delay
//                 {
//                     lastProcess->state = READY;
//                     kill(lastProcess->pid, SIGUSR1); // send signal to lastprocess to sleep
//                     lastProcess = pq->head->data;
//                     if (lastProcess->state == RUN || lastProcess->state == BLOCKED || lastProcess->state == READY)
//                     {
//                         kill(lastProcess->pid, SIGUSR2); // send signal to process to wakeup
//                     }
//                     else
//                     {
//                         int pid = fork();
//                         if (pid == 0)
//                         {
//                             lastProcess->state = RUN;
//                             char *args[] = {"./process.out", NULL};
//                             execvp(args[0], args);
//                         }
//                         else
//                         {
//                             lastProcess->startTime = getClk();
//                             lastProcess->mappedProcessID = getpid(); //@uncertain what do you mean by mappedProcessID
//                             lastProcess->pid = getpid();
//                         }
//                     }
//                 }
//             }
//             lastProcess->remainingTime--;
//         }
//     }
// }

int intializeMsgQueue(char *file, int num)
{
    key_t key_id;
    int rec_val, msgq_id;
    key_id = ftok(file, num);
    msgq_id = msgget(key_id, IPC_CREAT | 0666);
    if (msgq_id == -1)
    {
        perror("Error in creating message queue");
        exit(-1);
    }
    resources[currentResourcesCount++] = msgq_id;
    return msgq_id;
}

void intializeProcessRemainingTime(int remainingTime, int processId)
{
    int queueId = intializeMsgQueue(KEY_FILE, MSG_QUEUE_SHCEDULAR_PROCESS_KEY);
    messageBuff msg;
    msg.mIntegerData = remainingTime;
    msg.mtype = processId;
    int send_val = msgsnd(queueId, &msg, sizeof(msg.mIntegerData), !IPC_NOWAIT);
    if (send_val == -1)
    {
        perror("Errror in send");
    }
}

PCB *createProcess(Process *newProcess)
{
    PCB *processInstance = createPCBEntry(newProcess);
    int pid = fork();
    if (pid == 0)
    {
        // child process
        char *args[] = {"./process.out", NULL};
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else
    {
        /**
         * parent process:
         * 1. Stops the forked process
         * 2. assigns values in the process control block
         */
        kill(pid, SIGSTOP);
        processInstance->mappedProcessID = pid;
        intializeProcessRemainingTime(processInstance->remainingTime, pid);
    }
    return processInstance;
}

void closeIPCResources()
{
    for (int i = 0; i < currentResourcesCount; i++)
    {
        msgctl(resources[i], IPC_RMID, (struct msqid_ds *)0);
    }
}

void assignListToReference(void *list)
{
    switch (algorithmType)
    {
    case HPF:
        pqHPF = (pqueue_t *)list;
        break;
    case SRTN:
        pqSRTN = (pqueue_t *)list;
        break;
    case RR:
        llRR = (LinkedList *)list;
        break;
    }
}

void pqTest()
{
    PCB *processTable;
    pqueue_t *pq;
    processTable = malloc(MAX_NUM_OF_PROCESS * sizeof(PCB));
    processTable[0].priority = 0;
    processTable[0].processID = 0;
    processTable[1].priority = 1;
    processTable[1].processID = 1;
    processTable[2].priority = 2;
    processTable[2].processID = 2;
    processTable[3].priority = 3;
    processTable[3].processID = 3;
    processTable[4].priority = 4;
    processTable[4].processID = 4;
    pq = pqueue_init(MAX_NUM_OF_PROCESS, cmpPriority, getPriority, setPriority, get_pos, set_pos);
    pqueue_insert(pq, &processTable[0]);
    pqueue_insert(pq, &processTable[1]);
    pqueue_insert(pq, &processTable[2]);
    pqueue_insert(pq, &processTable[3]);
    pqueue_insert(pq, &processTable[4]);
    pqueue_change_priority(pq, 5, &processTable[0]);
    pqueue_change_priority(pq, 6, &processTable[1]);
    pqueue_change_priority(pq, 0, &processTable[2]);
    PCB *pqFront;
    while (pqFront = pqueue_pop(pq))
    {
        printf("Process ID: %d, Priority: %d\n", pqFront->processID, pqFront->priority);
    }
    pqueue_free(pq);
    free(processTable);
}

void *createHPFPQ()
{
    pqueue_t *pq;
    pq = pqueue_init(MAX_NUM_OF_PROCESS, cmpPriority, getPriority, setPriority, get_pos, set_pos);
    return pq;
}

void *createSRTNPQ()
{
    pqueue_t *pq;
    pq = pqueue_init(MAX_NUM_OF_PROCESS, cmpPriority, getRemainingTimeAsPriority, setRemainingTimeAsPriority, get_pos, set_pos);
    return pq;
}

void initProcessTable()
{
    processTable = malloc(MAX_NUM_OF_PROCESS * sizeof(PCB *));
}

void removeCurrentProcessFromDs()
{
    PCB *process;
    switch (algorithmType)
    {
    case HPF:
        printf("LST pcb %d\n", lstPCB->mappedProcessID);
        int res = pqueue_remove(pqHPF, (void *)lstPCB);
        if (res != 0)
        {
            printf("Removing from PQ fails");
        }
        process = lstPCB;
        break;
    case SRTN:
        process = pqueue_pop(pqSRTN);
        break;
    case RR:
        RemoveNode(llRR, lastNode);
        process = lastNode->data;
        break;
    }
    free(process);
    printf("Done Free\n");
}

static void handleProcesses(algorithm algorithm, addItem addToDS, createDS initDS)
{
    int rec_val, msgq_id;

    msgq_id = intializeMsgQueue(KEY_FILE, MSG_QUEUE_GENERATOR_SCHEDULAR_KEY);
    Process receivedProcess;
    void *list = initDS();
    assignListToReference(list);
    while (true)
    {
        rec_val = msgrcv(msgq_id, &receivedProcess, sizeof(Process) - sizeof(long), 0, IPC_NOWAIT);
        if (rec_val != -1)
        {
            printf("Received %d\n", receivedProcess.id);
            PCB *newPCBEntry = createProcess(&receivedProcess);
            printf("Process created with pid %d\n", newPCBEntry->mappedProcessID);
            addToDS(list, newPCBEntry);
        }
        algorithm(list);
    }
}

void childProcessTerminationHandler(int signum)
{
    int stat_loc = 0;
    int pid = wait(&stat_loc);
    printf("Yes sub-process %d is removed\n", pid);
    removeCurrentProcessFromDs();
    lstProcessKilled = pid;
}

void closeResources()
{
    closeIPCResources();
    destroyClk(true);
}

void processTerminationHandler(int signum)
{
    closeResources();
    killpg(getpgrp(), SIGKILL);
}

void initClkResource()
{
    initClk();
}

void initSigAction()
{
    struct sigaction sa;

    sa.sa_handler = childProcessTerminationHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_NOCLDSTOP;

    if (sigaction(SIGCHLD, &sa, NULL) == -1)
    {
        perror("Error in sigaction");
        exit(-1);
    }
}

void initSignalsHandlers()
{
    initSigAction();
    signal(SIGINT, processTerminationHandler);
}

void initSchedular()
{
    initClkResource();
    initProcessTable();
    initSignalsHandlers();
}

int main(int argc, char *argv[])
{
    initSchedular();
    algorithmType = HPF;

    printf("Schedular Id %d\n", getpid());
    switch (algorithmType)
    {
    case HPF:
        handleProcesses(processHPF, addToHPF, createHPFPQ);
        break;
    case SRTN:
        handleProcesses(processSRTN, addToSRTN, createSRTNPQ);
        break;
    case RR:
        handleProcesses(processRR, addToRR, createLL);
        break;
    }

    // any openened resources should be closed here
    closeResources();
}
