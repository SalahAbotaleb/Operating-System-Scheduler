#include "headers.h"
#include "Data_Structures/linked_list.h"
#include "Data_Structures/pqueue.h"

typedef u_int16_t ProcessID;
typedef u_int16_t Time;
typedef u_int16_t Priority;

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
    pid_t pid;

} PCB;

typedef void (*algorithm)(void *);
typedef void (*addItem)(PCB *);
typedef void *(*createDS)();

static void addTest(PCB *process)
{
    printf("Date: %d, %d, %d, %d\n", process->arrivalTime, process->remainingTime,
           process->priority, process->processID);
}

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

static void *createLL()
{
    return CreateLinkedList();
}

static void *createPQ()
{
    //    pqueue_init()
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

int lastclk = 0;
Node *lastNode = NULL;                                  // it is used in processRR & processSRTN
GHashTable* forkedSet = g_hash_table_new(g_str_hash, g_str_equal);

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
static void processHPF(void *pcbEntry)
{
    // 2 5 7 8
    // insertion
}
//

static void processSRTN(pqueue_t *pq)
{
    int clock = getClk();
    if (clock != lastclk)
    {
        lastclk = clock;
        if (pq->size > 0)
        {
            if (lastNode == NULL)
            {
                lastNode = pq->head;                        
            }
            else
            {
                // TODO stop current process
                // if (lastNode->nxt == NULL)
                // {
                //     lastNode = pq->head;
                // }
                // else
                // {
                //     lastNode = lastNode->nxt;
                // }
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
                process->finishTime = getClk();
                Node* finished_Node = pop(pq);
            }
            else
            {
                if (process != pq->head->data)                                      //@uncertain delay
                {
                    if(process->state != (BLOCKED || READY || RUN))
                    {
                        kill(process->pid, SIGUSER1);                               // send signal to process to wakeup
                    }
                    else
                    {
                        int pid = fork();
                        if (pid == 0)
                        {
                            char *args[] = {"./process.out", NULL};
                            execvp(args[0], args);
                        }
                        else
                        {
                            process->startTime = getClk();
                            process->mappedProcessID = pid;
                        }
                    }
                }
                

            }
        }
    }
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
    while (true)
    {
        // not sure of process size
        rec_val = msgrcv(msgq_id, &receivedProcess, sizeof(Process), 0, IPC_NOWAIT);
        if (rec_val != -1)
        {
            PCB *newPCBEntry = createPCBEntry(&receivedProcess);
            addToDS(list, newPCBEntry);
        }
        algorithm(list);
    }
}

int main(int argc, char *argv[])
{
    initClk();

    handleProcesses(processHPF, addTest, createPQ);
    handleProcesses(processSRTN, addToSRTN, createPQ);

    // TODO implement the scheduler :)
    // while (1) {
    // 1. read queue if there are any new processes
    // case (algorithm)
    // HFP:
    // addToHPF(pcbEntry)
    // SRTN:
    // addToSRTN(pcbEntry)
    // RR:
    // addToRR(pcbEntry)
    // check kol cycle
    // check lw galy signal enha 5lst

    // upon termination release the clock resources.

    destroyClk(true);
}
