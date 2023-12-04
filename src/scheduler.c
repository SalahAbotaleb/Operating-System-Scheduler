#include "headers.h"
#include "Data_Structures/linked_list.h"
#include "Data_Structures/pqueue.h"

typedef u_int16_t ProcessID;
typedef u_int16_t Time;
typedef u_int16_t Priority;


//
//typedef struct {
//    State state;
//    ProcessID processID;
//    ProcessID mappedProcessID;
//    Time arrivalTime;
//    Time startTime;
//    Time remainingTime;
//    Time finishTime;
//    Priority priority;
//
//} PCB;

typedef void (*Algorithm) (void *);
typedef void (*AlgorithmAddList) (Process *);
typedef void *(*AlgorithmCreateList) ();

//static void *createPEntry (Process *newProcess) {
//    Process *PEntry = malloc(sizeof(Process));
//    PEntry->id = newProcess->id;
//    PEntry->runTime = newProcess->runTime;
//    PEntry->remainingTime = newProcess->remainingTime;
//    PEntry->priority = newProcess->priority;
//    PEntry->arrivalTime = newProcess->arrivalTime;
//    return PEntry;
//}

//static void deletePCBEnrty (PCB *pcbEntry) {
//    free(pcbEntry);
//}
//
static void *createLL () {
    return CreateLinkedList();
}
static void *createPQ () {
//    pqueue_init()
}

static void addToRR (LinkedList *list, Process *Pentry) {
    AddNodeToBack(list, CreateNode(Pentry));
}


static void addToHPF (pqueue_t *pq, Process *Pentry) {
//    pqueue_insert(pq, Pentry);
}

static void addToSRTN (pqueue_t *pq, Process *Pentry) {
//    pqueue_insert(pq, Pentry);
    key_t key_id = ftok("keyfile", 90);               // create unique key
    int msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id
    
    message *send_val = malloc(sizeof(message));
    send_val->mtype = 1;
    send_val->mtext = "0";
    msgsnd(msgq_id, send_val, sizeof(message), !IPC_NOWAIT);
}

int lastclk = 0;
Node *lastNode = NULL;
static void processRR (LinkedList *list) {
    int clock = getClk();
    if (clock != lastclk) {
        lastclk = clock;
        if (list->size > 0) {
            if (lastNode == NULL) {
                lastNode = list->head;
            } else {
                // TODO stop current process
                if (lastNode->nxt == NULL) {
                    lastNode = list->head;
                } else {
                    lastNode = lastNode->nxt;
                }
            }
            Process *process = lastNode->data;
            process->remainingTime--;
            if (process->remainingTime == 0) {
                // TODO send message to process generator
                // TODO delete node
            } else {
                // TODO start process
            }
        }
    }
    
}
//
//static void processHPF (PCB *pcbEntry) {
//    // 2 5 7 8
//    // insertion
//}
//
//static void processSRTN (PCB *pcbEntry) {
//    // if as8ar call function handle
//}

static void handleProcesses (Algorithm algorithm, AlgorithmAddList addList, AlgorithmCreateList createList) {
    key_t key_id;
    int rec_val, msgq_id;
    
    key_id = ftok("keyfile", 65);               // create unique key
    msgq_id = msgget(key_id, 0666 | IPC_CREAT); // create message queue and return id
    
    if (msgq_id == -1) {
        perror("Error in create");
        exit(-1);
    }
    Process *receivedProcess = malloc(sizeof(Process));
    void *list = createList();
    while (true) {
        // not sure of process size
        rec_val = msgrcv(msgq_id, receivedProcess, sizeof(Process), 0, IPC_NOWAIT);
        if (rec_val != -1) {
            addList(receivedProcess);
        }
        algorithm(list);
    }
}

int main (int argc, char *argv[]) {
    initClk();
    
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
