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
static State lstState;
static int noProcessFinished = 0;
static int totalRunTime = 0;
static float totalWTA = 0;
static int totalWaitingTime = 0;
static float cpuUtilization = 0;
static float avgWTA = 0;
static float avgWaiting = 0;
static float stdWTA = 0;

void writeOutputPerfFile()
{
    FILE *file = fopen("scheduler.perf", "w");
    if (file == NULL)
    {
        perror("Error opening log file for writing");
        exit(EXIT_FAILURE);
    }
    cpuUtilization = ((float)totalRunTime / getClk()) * 100;
    avgWTA = totalWTA / noProcessFinished;
    avgWaiting = totalWaitingTime / noProcessFinished;
    // for (int i = 0; i < MAX_NUM_OF_PROCESS; i++)
    // {
    //     stdWTA += pow((processTable[i]->TA - avgWTA), 2);
    // }
    fprintf(file, "CPU utilization = %.2f%%\n", cpuUtilization);
    fprintf(file, "Avg WTA = %.2f\n", avgWTA);
    fprintf(file, "Avg Waiting = %.2f\n", avgWaiting);
    fprintf(file, "Std WTA = %.2f\n", stdWTA);

    fclose(file);
}

void writeOutputLogFile()
{
    FILE *file = fopen("scheduler.log", "w");
    if (file == NULL)
    {
        perror("Error opening log file for writing");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "#At time x process y state arr w total z remain y wait k\n");

    fclose(file);
}

void writeOutputLogFileStarted(PCB *process)
{
    if (process->startTime == getClk())
    {
        FILE *file = fopen("scheduler.log", "a");
        if (file == NULL)
        {
            perror("Error opening log file for writing");
            exit(EXIT_FAILURE);
        }
        fprintf(file, "At time %d process %d Started arr %d total %d remain %d wait %d\n",
                getClk(),
                process->processID,
                process->arrivalTime,
                process->runTime,
                process->remainingTime,
                process->wait);

        fclose(file);
    }
}

void writeOutputLogFileResumed(PCB *process)
{
    if (algorithmType != HPF)
    {
        FILE *file = fopen("scheduler.log", "a");
        if (file == NULL)
        {
            perror("Error opening log file for writing");
            exit(EXIT_FAILURE);
        }
        fprintf(file, "At time %d process %d Resumed arr %d total %d remain %d wait %d\n",
                getClk(),
                process->processID,
                process->arrivalTime,
                process->runTime,
                process->remainingTime,
                process->wait);

        fclose(file);
    }
}

void writeOutputLogFileStopped(PCB *process)
{
    FILE *file = fopen("scheduler.log", "a");
    if (file == NULL)
    {
        perror("Error opening log file for writing");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "At time %d process %d Stopped arr %d total %d remain %d wait %d\n",
            getClk(),
            process->processID,
            process->arrivalTime,
            process->runTime,
            process->remainingTime,
            process->wait);

    fclose(file);
}

void writeOutputLogFileFinished(PCB *process)
{
    FILE *file = fopen("scheduler.log", "a");
    if (file == NULL)
    {
        perror("Error opening log file for writing");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "At time %d process %d Finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
            getClk(),
            process->processID,
            process->arrivalTime,
            process->runTime,
            process->remainingTime, // the remaining time is not correct as it is not updated from processor (zahar)
            process->wait,
            process->TA,
            (float)(process->TA) / process->runTime);

    fclose(file);
}

void stopProcess(PCB *process)
{
    writeOutputLogFileStopped(process);
    kill(process->mappedProcessID, SIGSTOP);
}

void contiuneProcess(PCB *process)
{
    if (process->startTime != getClk())
        writeOutputLogFileResumed(process);
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
    newPCBEntry->runTime = newProcess->runTime; // needed for output files (zahar)
    newPCBEntry->startTime = -1;                // to know if it was excuted before
    return newPCBEntry;
}

static void setPCBStartTime(PCB *pcbEntry)
{
    if (pcbEntry->startTime == -1) // it is working as a flag but i prefer to use a boolean (zahar)
    {
        pcbEntry->startTime = getClk();
        printf("Start time %d\n", pcbEntry->startTime);
        pcbEntry->wait = pcbEntry->startTime - pcbEntry->arrivalTime;
        totalWaitingTime += pcbEntry->wait;
    }
}

static void setPCBFinishedTime(PCB *pcbEntry)
{
    pcbEntry->finishTime = getClk();
    pcbEntry->TA = pcbEntry->finishTime - pcbEntry->arrivalTime;
    totalWTA = totalWTA + ((float)pcbEntry->TA / pcbEntry->runTime);
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
    static Time lstTime = -1;

    PCB *highestPriorityProcess = pqueue_peek(pq);
    if (highestPriorityProcess == NULL)
    {
        return;
    }
    if (currProcess == -1)
    {
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    }
    else if (lstProcessKilled == currProcess)
    {
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    }

    Time currTime = getClk();
    if (lstTime != currTime && currTime != lstPCB->startTime)
    {
        printf("Prev %d Curr %d\n", lstTime, currTime);
        lstTime = currTime;
        lstPCB->remainingTime--;
    }
}

static void processSRTN(void *pqT)
{
    pqueue_t *pq = pqT;
    // should continue function here
    // TODO: copy commented part and pase here
    if (pq->size <= 1)
        return;

    static ProcessID currProcess = -1;
    static Time lstTime = -1;

    PCB *highestPriorityProcess = pqueue_peek(pq);
    if (highestPriorityProcess == NULL)
    {
        return;
    }

    if (currProcess == -1)
    {
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    }
    else if (lstProcessKilled == currProcess)
    {
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    }
    else if (lstPCB != highestPriorityProcess)
    {
        stopProcess(lstPCB);
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    }
    Time currTime = getClk();
    if (lstTime != currTime && currTime != lstPCB->startTime)
    {
        lstTime = currTime;
        lstPCB->remainingTime--;
    }
}

//zahar is cooking
// static void processRR(void *listT)
// {
//     LinkedList *list = (LinkedList *)listT;
//     if (list->size == 0)
//         return;

//     static ProcessID currProcess = -1;
//     static Time lstTime = -1;

//     PCB *headProcess = Peek(list);
//     if (headProcess == NULL)
//     {
//         return;
//     }
    
//     if(currProcess == -1)
//     {
//         currProcess = headProcess->mappedProcessID;
//         lstPCB = headProcess;
//         setPCBStartTime(lstPCB);
//         writeOutputLogFileStarted(lstPCB);
//         contiuneProcess(lstPCB);
//     }
//     else if (lstProcessKilled == currProcess)
//     {
//        // if(list->nxt == NULL)
//        //     lstPCB = headProcess;
//        // else
//        //     lstPCB = list->nxt->data;
//         currProcess = lstPCB->mappedProcessID;
//         setPCBStartTime(lstPCB);
//         writeOutputLogFileStarted(lstPCB);
//         contiuneProcess(lstPCB);
//     }
    
//     ///////////////////
//     // else if(quantum condition)
//     ////////////////////

//     Time currTime = getClk();
//     if (lstTime != currTime && currTime != lstPCB->startTime)
//     {
//         lstTime = currTime;
//         lstPCB->remainingTime--;
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

int shedularProcessQueueId = 0;
void intializeProcessRemainingTime(int remainingTime, int processId)
{
    messageBuff msg;
    msg.mIntegerData = remainingTime;
    msg.mtype = processId;
    int send_val = msgsnd(shedularProcessQueueId, &msg, sizeof(msg.mIntegerData), IPC_NOWAIT);
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
        totalRunTime += processInstance->runTime;
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
    setPCBFinishedTime(process);
    writeOutputLogFileFinished(process);
    free(process);
    noProcessFinished++;
    printf("Done Free\n");
}

int generatorSchedularQueueId = 0;
static void handleProcesses(algorithm algorithm, addItem addToDS, createDS initDS)
{
    int rec_val;

    Process receivedProcess;
    void *list = initDS();
    assignListToReference(list);
    writeOutputLogFile();
    while (true)
    {
        rec_val = msgrcv(generatorSchedularQueueId, &receivedProcess, sizeof(Process) - sizeof(long), 0, IPC_NOWAIT);
        if (rec_val != -1)
        {
            printf("Received %d\n", receivedProcess.id);
            PCB *newPCBEntry = createProcess(&receivedProcess);
            printf("Process created with pid %d\n", newPCBEntry->mappedProcessID);
            addToDS(list, newPCBEntry);
        }
        algorithm(list);
        if (noProcessFinished == MAX_NUM_OF_PROCESS) // 3 should be replaced with acrual # of processes from process_generator (zahar)
        {
            writeOutputPerfFile();
            break;
        }
    }
}

void childProcessTerminationHandler(int signum)
{
    int stat_loc = 0;
    printf("Waittt\n");
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
    shedularProcessQueueId = intializeMsgQueue(KEY_FILE, MSG_QUEUE_SHCEDULAR_PROCESS_KEY);
    generatorSchedularQueueId = intializeMsgQueue(KEY_FILE, MSG_QUEUE_GENERATOR_SCHEDULAR_KEY);
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
