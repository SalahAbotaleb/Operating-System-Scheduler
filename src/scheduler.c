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
    //    pqueue_insert(pq, Pentry);
    key_t key_id = ftok("keyfile", 90);             // create unique key
    int msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id

    message *send_val = malloc(sizeof(message));
    send_val->mtype = 1;
    send_val->mtext = "0";
    msgsnd(msgq_id, send_val, sizeof(message), !IPC_NOWAIT);
}

int lastclk = 0;
Node *lastNode = NULL;
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
// static void processSRTN (PCB *pcbEntry) {
//    // if as8ar call function handle
//}

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
            addToDS(newPCBEntry);
        }
        algorithm(list);
    }
}

int main(int argc, char *argv[])
{
    initClk();

    handleProcesses(processHPF, addTest, createPQ);
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
