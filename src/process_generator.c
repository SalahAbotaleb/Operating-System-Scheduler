#include "headers.h"

void clearResources (int);
Process *readInputFiles ();
SchedulingAlgorithm chooseSchedulingAlgorithm ();
static long quantum = 0;

int main (int argc, char *argv[]) {
    //    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    Process *processes;
    processes = readInputFiles();
    
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    SchedulingAlgorithm algorithm = chooseSchedulingAlgorithm();
    
    // 3. Initiate and create the scheduler and clock processes.
    pid_t clockPid = fork();
    if (clockPid == 0) {
        // clock process
        char *args[] = {"./clk.out", NULL};
        execvp(args[0], args);
    }
    
    pid_t schedulerPid = fork();
    if (schedulerPid == 0) {
        // scheduler process
        char alg[2];
        sprintf(alg, "%d", algorithm);
        char quant[2];
        sprintf(quant, "%ld", quantum);
        char *args[] = {"./scheduler.out", alg, quant, NULL};
        execvp(args[0], args);
    }
    
    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int intialTime = getClk();
    printf("current time is %d\n", intialTime);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // Done
    
    // 6. Send the information to the scheduler at the appropriate time.
    int currProcess = 0;
    key_t key_id = ftok(KEY_FILE, MSG_QUEUE_GENERATOR_SCHEDULAR_KEY); // create unique key
    int msgq_id = msgget(key_id, 0666 | IPC_CREAT);                   // create message queue and return id
    while (currProcess < MAX_NUM_OF_PROCESS) {
        int currTime = getClk();
        while (currTime >= processes[currProcess].arrivalTime && currProcess < MAX_NUM_OF_PROCESS) {
            // TODO send the process to the scheduler
            processes[currProcess].mtype = 1;
            int send_val = msgsnd(msgq_id, &processes[currProcess], sizeof(Process) - sizeof(long), !IPC_NOWAIT);
            if (send_val == -1) {
                perror("Error in send");
                continue;
            }
            
            printf("Sent process %d to scheduler\n", processes[currProcess].id);
            currProcess++;
        }
    }
    while (1) {
    }
    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources (int signum) {
    // TODO Clears all resources in case of interruption
}

/** @brief Reads the input files and returns a 2D array of processes
 * @return 2D array of processes
 */
Process *readInputFiles () {
    FILE *file = fopen("processes.txt", "r");
    char *line = malloc(100 * sizeof(char));
    
    Process *processes = malloc(MAX_NUM_OF_PROCESS * sizeof(Process)); // not dynamic
    
    int i = 0;
    while (fgets(line, 100, file)) {
        if (line[0] == '#')
            continue;
        
        char *token = strtok(line, "\t");
        
        processes[i].id = atoi(token);
        token = strtok(NULL, "\t");
        processes[i].arrivalTime = atoi(token);
        token = strtok(NULL, "\t");
        processes[i].runTime = atoi(token);
        token = strtok(NULL, "\t");
        processes[i].priority = atoi(token);
        
        i++;
    }
    
    return processes;
}

// @brief Asks the user for the chosen scheduling algorithm and its parameters, if there are any.
SchedulingAlgorithm chooseSchedulingAlgorithm () {
    int algorithm;
    printf("Please choose the scheduling algorithm:\n");
    printf("1- HPF\n");
    printf("2- SRTN\n");
    printf("3- RR\n");
    do {
        printf(">> ");
        scanf("%d", &algorithm);
    } while (algorithm < 1 || algorithm > 3);
    algorithm--;
    switch (algorithm) {
        case RR:
            printf("Please enter the time quantum:\n");
            do {
                printf(">> ");
                scanf("%ld", &quantum);
            } while (quantum < 1);
            break;
        
    }
    
    
    return algorithm;
}