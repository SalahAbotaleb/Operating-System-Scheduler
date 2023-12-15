#include "headers.h"
#include "Data_Structures/linked_list.h"
#include "Data_Structures/pqueue.h"
#include "pqCustomizedCB.c"
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>

typedef void (*algorithm) (void *);
typedef void (*addItem) (void *, PCB *);
typedef void *(*createDS) ();

static int lastclk = 0;
static int lastsec = 0;
static Node *lastNode = NULL; // it is used in processRR & processSRTN
static PCB **processTable = NULL;
static SchedulingAlgorithm algorithmType;
static pqueue_t *pqHPF = NULL;  // priority queue for Highest Priority First Algorithm
static pqueue_t *pqSRTN = NULL; // priority queue for Shortet Remaining Time Next Algorithm
static LinkedList *llRR = NULL; // linked list for Round Robin Algorithm
static int resources[MAX_NUM_OF_RESOURCES] = {-1};
static int currentResourcesCount = 0;
static int lstProcessKilled = -2;
static int isProcessKilled = 0;
static int isProcessRemoved = 0;
static PCB *lstPCB = NULL;
static State lstState;
static int noProcessFinished = 0;
static int totalRunTime = 0;
static float totalWTA = 0;
static float totalWaitingTime = 0;
static float cpuUtilization = 0;
static float totalTA = 0;
static float avgWTA = 0;
static float avgWaiting = 0;
static float stdWTA = 0;
static float stdWTAsq = 0;
int quantum = 1; //process quantum send by process generator in argument 
int currquantum; //current quantum for process
int maxNumOfProcess = 1; //max number of process send by process generator in argument

void initializeQuantum () {
    currquantum = quantum;
}

void writeOutputPerfFile () {
    FILE *file = fopen("scheduler.perf", "w");
    if (file == NULL) {
        perror("Error opening log file for writing");
        exit(EXIT_FAILURE);
    }
    cpuUtilization = ((float) totalRunTime / getClk()) * 100;
    avgWTA = totalWTA / noProcessFinished;
    avgWaiting = totalWaitingTime / noProcessFinished;
    stdWTA = sqrt((stdWTAsq / noProcessFinished) - (avgWTA * avgWTA));
    fprintf(file, "CPU utilization = %.2f%%\n", cpuUtilization);
    fprintf(file, "Avg WTA = %.2f\n", avgWTA);
    fprintf(file, "Avg Waiting = %.2f\n", avgWaiting);
    fprintf(file, "Std WTA = %.2f\n", stdWTA);
    
    fclose(file);
}

void writeOutputLogFile () {
    FILE *file = fopen("scheduler.log", "w");
    if (file == NULL) {
        perror("Error opening log file for writing");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "#At time x process y state arr w total z remain y wait k\n");
    
    fclose(file);
}

void writeOutputLogFileStarted (PCB *process) {
    if (process->startTime == getClk()) {
        FILE *file = fopen("scheduler.log", "a");
        if (file == NULL) {
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

void writeOutputLogFileResumed (PCB *process) {
    if (algorithmType != HPF) {
        FILE *file = fopen("scheduler.log", "a");
        if (file == NULL) {
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

void writeOutputLogFileStopped (PCB *process) {
    FILE *file = fopen("scheduler.log", "a");
    if (file == NULL) {
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

void writeOutputLogFileFinished (PCB *process) {
    FILE *file = fopen("scheduler.log", "a");
    if (file == NULL) {
        perror("Error opening log file for writing");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "At time %d process %d Finished arr %d total %d remain %d wait %d TA %d WTA %.2f\n",
            getClk(),
            process->processID,
            process->arrivalTime,
            process->runTime,
            process->remainingTime, 
            process->wait,
            process->TA,
            process->WTA);
    
    fclose(file);
}

void stopProcess (PCB *process) {
    writeOutputLogFileStopped(process);
    kill(process->mappedProcessID, SIGSTOP);
}

void contiuneProcess (PCB *process) {
    if (process->startTime != getClk())
    {
        process->wait = getClk() - process->arrivalTime - process->runTime + process->remainingTime;
        writeOutputLogFileResumed(process);
    }
    kill(process->mappedProcessID, SIGCONT);
}

static void addTest (PCB *process) {
    printf("Date: %d, %d, %d, %d\n", process->arrivalTime, process->remainingTime,
           process->priority, process->processID);
}

static PCB *createPCBEntry (Process *newProcess) {
    PCB *newPCBEntry = (PCB *) malloc(sizeof(PCB));
    newPCBEntry->arrivalTime = newProcess->arrivalTime;
    newPCBEntry->remainingTime = newProcess->runTime;
    newPCBEntry->priority = newProcess->priority;
    newPCBEntry->processID = newProcess->id;
    newPCBEntry->runTime = newProcess->runTime; 
    newPCBEntry->startTime = -1;                // to know if it was excuted before
    return newPCBEntry;
}

static void setPCBStartTime (PCB *pcbEntry) {
    if (pcbEntry->startTime == -1)
    {
        pcbEntry->startTime = getClk();
        printf("clock %d\n", getClk());
        printf("Start time %d\n", pcbEntry->startTime);
        if (pcbEntry->startTime == 0)
            pcbEntry->wait = 0;
        else
            pcbEntry->wait = pcbEntry->startTime - pcbEntry->arrivalTime;
    }
}

static void setPCBFinishedTime (PCB *pcbEntry) {
    pcbEntry->finishTime = getClk();
    pcbEntry->TA = pcbEntry->finishTime - pcbEntry->arrivalTime;
    pcbEntry->wait = pcbEntry->TA - pcbEntry->runTime;
    totalWaitingTime += pcbEntry->wait;
    pcbEntry->WTA = (float) pcbEntry->TA / pcbEntry->runTime;
    totalWTA = totalWTA + pcbEntry->WTA;
    stdWTAsq += (pcbEntry->WTA * pcbEntry->WTA);
    totalTA += pcbEntry->TA;
}

static void deletePCBEnrty (PCB *pcbEntry) {
    free(pcbEntry);
}

static void *createLL () {
    return CreateLinkedList();
}

static void addToRR (void *listT, PCB *pEntry) {
    LinkedList *list = (LinkedList *) listT;
    AddNodeToBack(list, CreateNode(pEntry));
}

static void addToHPF (void *pqT, PCB *pEntry) {
    pqueue_t *pq = (pqueue_t *) pqT;
    pqueue_insert(pq, pEntry);
}

static void addToSRTN (void *pqT, PCB *pEntry) {
    pqueue_t *pq = (pqueue_t *) pqT;
    pqueue_insert(pq, pEntry);
}

static void processRR (void *listT) {
    LinkedList *list = (LinkedList *) listT;
    if (list->size == 0)
        return;
    
    if (list->head == NULL) {
        return;
    }
    static Time lstTime = -1;
    
    int clock = getClk();
    
    if (isProcessRemoved == 1) {
        
        lastclk = clock;
        lastsec = clock;
        
        if (lastNode->nxt == NULL) {
            lastNode = list->head;
        } else {
            lastNode = lastNode->nxt;
        }
        //printf("Process %d should start\n", ((PCB *) lastNode->data)->processID);
        PCB *process = lastNode->data;
        setPCBStartTime(process);
        //printf("after set start time\n");
        writeOutputLogFileStarted(process);
        //printf("after write to log file\n");
        contiuneProcess(process);
        //printf("after continue process\n");
        printf("last clk %d\n", lastclk);
        isProcessRemoved = 0;
    } else if ((clock - lastclk >= quantum && lastNode != NULL && ((PCB *) lastNode->data)->remainingTime > quantum) || lastNode == NULL) {
        if (lastNode == NULL) {
            lastNode = list->head;
            if (lastNode == NULL) {
                return;
            }
        } else {
            lstPCB = lastNode->data;
            
            //printf("Prev %d Curr %d\n", lastclk, clock);
            lstPCB->remainingTime = lstPCB->remainingTime - quantum;
            
            stopProcess(lstPCB);
            //print linked list
            Node *temp = list->head;
            while (temp != NULL) {
                printf("Process ID: %d, Priority: %d\n", ((PCB *) temp->data)->processID,
                       ((PCB *) temp->data)->priority);
                temp = temp->nxt;
            }
            
            if (lastNode->nxt == NULL) {
                lastNode = list->head;
            } else {
                lastNode = lastNode->nxt;
            }
        }
        lastclk = clock;
        lastsec = clock;
        printf("Last clk %d\n", lastclk);
        
        PCB *process = lastNode->data;
        
        setPCBStartTime(process);
        printf("after set start time\n");
        writeOutputLogFileStarted(process);
        printf("after write to log file\n");
        contiuneProcess(process);
        printf("after continue process\n");
        printf("last clk %d\n", lastclk);
    } else if (clock != lastsec && ((PCB *) lastNode->data)->remainingTime <= quantum &&
               ((PCB *) lastNode->data)->remainingTime > 0) {
        ((PCB *) lastNode->data)->remainingTime--;
        lastsec = clock;
    }
}

static void processHPF (void *pqT) {
    pqueue_t *pq = pqT;
    if (pq->size <= 1)
        return;
    
    static ProcessID currProcess = -1;
    static Time lstTime = -1;
    
    PCB *highestPriorityProcess = pqueue_peek(pq);
    if (highestPriorityProcess == NULL) {
        return;
    }
    if (currProcess == -1) {
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    } else if (isProcessRemoved == 1) {
        isProcessRemoved = 0;
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    }
    
    Time currTime = getClk();
    if (lstTime != currTime && currTime != lstPCB->startTime) {
        printf("Prev %d Curr %d\n", lstTime, currTime);
        lstTime = currTime;
        lstPCB->remainingTime--;
    }
}

static void processSRTN (void *pqT) {
    pqueue_t *pq = pqT;
    // should continue function here
    // TODO: copy commented part and pase here
    if (pq->size <= 1)
        return;
    
    static ProcessID currProcess = -1;
    static Time lstTime = -1;
    
    PCB *highestPriorityProcess = pqueue_peek(pq);
    if (highestPriorityProcess == NULL) {
        return;
    }
    Time currTime = getClk();
    int dec = 0;
    if (currProcess == -1) {
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    } else if (isProcessRemoved == 1) {
        isProcessRemoved = 0;
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    } else if (lstPCB != highestPriorityProcess) {
        if (lstTime != currTime && currTime != lstPCB->startTime) {
            lstTime = currTime;
            lstPCB->remainingTime--;
            dec = 1;
        }
        stopProcess(lstPCB);
        currProcess = highestPriorityProcess->mappedProcessID;
        lstPCB = highestPriorityProcess;
        setPCBStartTime(lstPCB);
        writeOutputLogFileStarted(lstPCB);
        contiuneProcess(lstPCB);
    }
    if (lstTime != currTime && currTime != lstPCB->startTime) {
        lstTime = currTime;
        if (dec == 0)
            lstPCB->remainingTime--;
        dec = 0;
    }
}

int intializeMsgQueue (char *file, int num) {
    key_t key_id;
    int rec_val, msgq_id;
    key_id = ftok(file, num);
    msgq_id = msgget(key_id, IPC_CREAT | 0666);
    if (msgq_id == -1) {
        perror("Error in creating message queue");
        exit(-1);
    }
    resources[currentResourcesCount++] = msgq_id;
    return msgq_id;
}

int shedularProcessQueueId = 0;
void intializeProcessRemainingTime (int remainingTime, int processId) {
    messageBuff msg;
    msg.mIntegerData = remainingTime;
    msg.mtype = processId;
    int send_val = msgsnd(shedularProcessQueueId, &msg, sizeof(msg.mIntegerData), IPC_NOWAIT);
    if (send_val == -1) {
        perror("Errror in send");
    }
}

PCB *createProcess (Process *newProcess) {
    PCB *processInstance = createPCBEntry(newProcess);
    int pid = fork();
    if (pid == 0) {
        // child process
        char *args[] = {"./process.out", NULL};
        execvp(args[0], args);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
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

void closeIPCResources () {
    for (int i = 0; i < currentResourcesCount; i++) {
        msgctl(resources[i], IPC_RMID, (struct msqid_ds *) 0);
    }
}

void assignListToReference (void *list) {
    switch (algorithmType) {
        case HPF:
            pqHPF = (pqueue_t *) list;
            break;
        case SRTN:
            pqSRTN = (pqueue_t *) list;
            break;
        case RR:
            llRR = (LinkedList *) list;
            break;
    }
}

void pqTest () {
    PCB *processTable;
    pqueue_t *pq;
    processTable = malloc(maxNumOfProcess * sizeof(PCB));
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
    pq = pqueue_init(maxNumOfProcess, cmpPriority, getPriority, setPriority, get_pos, set_pos);
    pqueue_insert(pq, &processTable[0]);
    pqueue_insert(pq, &processTable[1]);
    pqueue_insert(pq, &processTable[2]);
    pqueue_insert(pq, &processTable[3]);
    pqueue_insert(pq, &processTable[4]);
    pqueue_change_priority(pq, 5, &processTable[0]);
    pqueue_change_priority(pq, 6, &processTable[1]);
    pqueue_change_priority(pq, 0, &processTable[2]);
    PCB *pqFront;
    while (pqFront = pqueue_pop(pq)) {
        printf("Process ID: %d, Priority: %d\n", pqFront->processID, pqFront->priority);
    }
    pqueue_free(pq);
    free(processTable);
}

void *createHPFPQ () {
    pqueue_t *pq;
    pq = pqueue_init(maxNumOfProcess, cmpPriority, getPriority, setPriority, get_pos, set_pos);
    return pq;
}

void *createSRTNPQ () {
    pqueue_t *pq;
    pq = pqueue_init(maxNumOfProcess, cmpPriority, getRemainingTimeAsPriority, setRemainingTimeAsPriority, get_pos,
                     set_pos);
    return pq;
}

void initProcessTable () {
    processTable = malloc(maxNumOfProcess * sizeof(PCB *));
}

void removeCurrentProcessFromDs () {
    PCB *process;
    switch (algorithmType) {
        case HPF:
            int res = pqueue_remove(pqHPF, (void *) lstPCB);
            if (res != 0) {
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
}

int generatorSchedularQueueId = 0;

static void handleProcesses (algorithm algorithm, addItem addToDS, createDS initDS) {
    int rec_val;
    
    Process receivedProcess;
    void *list = initDS();
    assignListToReference(list);
    writeOutputLogFile();
    int lstclk = -1;
    while (true) {
        if(algorithmType == RR)
        {
            if (getClk() == lstclk)
                usleep(50);
            lstclk = getClk();
        }
        if (isProcessKilled == 1) {
            isProcessKilled = 0;
            removeCurrentProcessFromDs();
            isProcessRemoved = 1;
        }
        rec_val = msgrcv(generatorSchedularQueueId, &receivedProcess, sizeof(Process) - sizeof(long), 0, IPC_NOWAIT);
        if (rec_val != -1) {
            printf("Received %d\n", receivedProcess.id);
            PCB *newPCBEntry = createProcess(&receivedProcess);
            printf("Process created with pid %d\n", newPCBEntry->mappedProcessID);
            addToDS(list, newPCBEntry);
        }
        algorithm(list);

        if (noProcessFinished == maxNumOfProcess) 
        {
            writeOutputPerfFile();
            printf("avgTA: %.2f\n", totalTA / noProcessFinished);
            break;
        }
    }
}


void childProcessTerminationHandler (int signum) {
    int stat_loc = 0;
    int pid = wait(&stat_loc);
    printf("Yes sub-process %d is removed\n", pid);
    isProcessKilled = 1;
    //lstProcessKilled = pid;
}

void closeResources () {
    closeIPCResources();
    destroyClk(true);
}

void processTerminationHandler (int signum) {
    closeResources();
    killpg(getpgrp(), SIGKILL);
}

void initClkResource () {
    initClk();
}

void initSigAction () {
    struct sigaction sa;
    
    sa.sa_handler = childProcessTerminationHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO | SA_NOCLDSTOP;
    
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error in sigaction");
        exit(-1);
    }
}

void initSignalsHandlers () {
    initSigAction();
    signal(SIGINT, processTerminationHandler);
}

void initSchedular () {
    initClkResource();
    initProcessTable();
    initSignalsHandlers();
    shedularProcessQueueId = intializeMsgQueue(KEY_FILE, MSG_QUEUE_SHCEDULAR_PROCESS_KEY);
    generatorSchedularQueueId = intializeMsgQueue(KEY_FILE, MSG_QUEUE_GENERATOR_SCHEDULAR_KEY);
}

int main (int argc, char *argv[]) {
    initSchedular();
    algorithmType = atoi(argv[1]);
    quantum = atoi(argv[2]);
    maxNumOfProcess = atoi(argv[3]);
    
    //printf("Schedular Id %d\n", getpid());
    switch (algorithmType) {
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
