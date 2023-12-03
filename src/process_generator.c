#include "headers.h"

typedef enum SchedulingAlgorithms
{
    HPF,
    SRTN,
    RR
} SchedulingAlgorithm;

typedef struct Process
{
    int id;
    int arrivalTime;
    int runTime;
    int priority;
} Process;

void clearResources(int);
Process *readInputFiles();
SchedulingAlgorithm chooseSchedulingAlgorithm();

int main(int argc, char *argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    Process *processes;
    processes = readInputFiles();

    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    SchedulingAlgorithm algorithm = chooseSchedulingAlgorithm();

    // 3. Initiate and create the scheduler and clock processes.
    pid_t schedulerPid = fork();
    if (schedulerPid == 0)
    {
        // scheduler process
        char *args[] = {"./scheduler", NULL};
        execvp(args[0], args);
    }

    pid_t clockPid = fork();
    if (clockPid == 0)
    {
        // clock process
        char *args[] = {"./clk", NULL};
        execvp(args[0], args);
    }

    // 4. Use this function after creating the clock process to initialize clock
    initClk();
    // To get time use this
    int x = getClk();
    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // Done

    // 6. Send the information to the scheduler at the appropriate time.
    int i = 0;
    while (processes[i].id != 0)
    {
        x = getClk();
        while (x == processes[i].arrivalTime)
        {
            // TODO send the process to the scheduler
            i++;
        }
    }

    // 7. Clear clock resources
    destroyClk(true);
}

void clearResources(int signum)
{
    // TODO Clears all resources in case of interruption
}

/** @brief Reads the input files and returns a 2D array of processes
 * @return 2D array of processes
 */
Process *readInputFiles()
{
    FILE *file = fopen("processes.txt", "r");
    char *line = malloc(100 * sizeof(char));

    Process *processes = malloc(20 * sizeof(Process));

    int i = 0;
    while (fgets(line, 100, file))
    {
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
        printf("%d\t%d\t%d\t%d\n", processes[i].id, processes[i].arrivalTime, processes[i].runTime,
               processes[i].priority);
        i++;
    }

    return processes;
}

// @brief Asks the user for the chosen scheduling algorithm and its parameters, if there are any.
SchedulingAlgorithm chooseSchedulingAlgorithm()
{
    int algorithm;
    printf("Please choose the scheduling algorithm:\n");
    printf("1- HPF\n");
    printf("2- SRTN\n");
    printf("3- RR\n");
    do
    {
        printf(">> ");
        scanf("%d", &algorithm);
    } while (algorithm < 1 || algorithm > 3);

    return algorithm - 1;
}