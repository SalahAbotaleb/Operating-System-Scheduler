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
typedef void (*addItem)(PCB *);
typedef void *(*createDS)();

static int lastclk = 0;
static Node *lastNode = NULL; // it is used in processRR & processSRTN
static PCB **processTable = NULL;
static SchedulingAlgorithm algorithmType;
static pqueue_t *pqHPF = NULL;  // priority queue for Highest Priority First Algorithm
static pqueue_t *pqSRTN = NULL; // priority queue for Shortet Remaining Time Next Algorithm
static LinkedList *llRR = NULL; // linked list for Round Robin Algorithm

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

static void *createPQ()
{
    // pqueue_init();
}

static void addToRR(LinkedList *list, PCB *pEntry)
{
    AddNodeToBack(list, CreateNode(pEntry));
}

static void addToHPF(pqueue_t *pq, PCB *pEntry)
{
    //    pqueue_insert(pq, pEntry);
}

static void addToSRTN(pqueue_t *pq, PCB *pEntry)
{
    int success = pqueue_insert(pq, pEntry);
}

static void processRR(LinkedList *list)
{
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
//
static void processHPF(pqueue_t *pq)
{
    // 2 5 7 8
    // insertion
}
//

static void processSRTN(pqueue_t *pq)
{
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

PCB *createProcess(Process *newProcess)
{
    PCB *processInstance = createPCBEntry(newProcess);
    int pid = fork();
    if (pid == 0)
    {
        // child process
        execv("./process.out", NULL);
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
    }
    return processInstance;
}

void stopProcess(PCB *process)
{
    kill(process->mappedProcessID, SIGSTOP);
}

void contiuneProcess(PCB *process)
{
    kill(process->mappedProcessID, SIGCONT);
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

pqueue_t *createHPFPQ()
{
    pqueue_t *pq;
    pq = pqueue_init(MAX_NUM_OF_PROCESS, cmpPriority, getPriority, setPriority, get_pos, set_pos);
    return pq;
}

pqueue_t *createSRTNPQ()
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
        process = pqueue_pop(pqHPF);
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
}

static void handleProcesses(algorithm algorithm, addItem addToDS, createDS initDS)
{
    key_t key_id;
    int rec_val, msgq_id;

    key_id = ftok("keyfile", 65);               // create unique key
    msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

    if (msgq_id == -1)
    {
        perror("Error in create");
        exit(-1);
    }
    Process receivedProcess;
    void *list = initDS();
    assignListToReference(list);
    while (true)
    {
        // not sure of process size
        rec_val = msgrcv(msgq_id, &receivedProcess, sizeof(Process), 0, IPC_NOWAIT);
        if (rec_val != -1)
        {
            PCB *newPCBEntry = createPCBEntry(&receivedProcess);
            addToDS(newPCBEntry); // it should be addToDS(list, newPCBEntry);
        }
        algorithm(list);
    }
}

void processTerminationHandler(int signum)
{
    int stat_loc = 0;
    int pid = wait(&stat_loc);
    removeCurrentProcessFromDs();
}

int main(int argc, char *argv[])
{
    initClk();
    initProcessTable();
    signal(SIGCHLD, processTerminationHandler);

    // handleProcesses(processHPF, addTest, createPQ);
    // // handleProcesses(processSRTN, addToSRTN, createPQ);

    // // TODO implement the scheduler :)
    // // while (1) {
    // // 1. read queue if there are any new processes
    // // case (algorithm)
    // // HFP:
    // // addToHPF(pcbEntry)
    // // SRTN:
    // // addToSRTN(pcbEntry)
    // // RR:
    // // addToRR(pcbEntry)
    // // check kol cycle
    // // check lw galy signal enha 5lst

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
    // upon termination release the clock resources.
    destroyClk(true);
}
