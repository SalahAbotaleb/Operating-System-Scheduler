#include "headers.h"

void clearResources(int);
int **readInputFiles();
enum SchedulingAlgorithms {HPF, SRTN, RR};

int main(int argc, char * argv[])
{
    signal(SIGINT, clearResources);
    // TODO Initialization
    // 1. Read the input files.
    int **processes;
    processes = readInputFiles();
    // 2. Ask the user for the chosen scheduling algorithm and its parameters, if there are any.
    
    // 3. Initiate and create the scheduler and clock processes.
    // 4. Use this function after creating the clock process to initialize clock
//    initClk();
    // To get time use this
//    int x = getClk();
//    printf("current time is %d\n", x);
    // TODO Generation Main Loop
    // 5. Create a data structure for processes and provide it with its parameters.
    // 6. Send the information to the scheduler at the appropriate time.
    // 7. Clear clock resources
//    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}

int **readInputFiles()
{
    FILE *file = fopen("processes.txt", "r");
    char *line = malloc(100 * sizeof(char));
    int **processes = malloc(20 * sizeof(int*));
    for (int i = 0; i < 20; ++i) {
        processes[i] = malloc(4 * sizeof(int));
    }
    int i = 0;
    while (fgets(line, 100, file))
    {
        if (line[0] == '#')
            continue;
        char *token = strtok(line, "\t");
        int j = 0;
        while (token != NULL)
        {
            processes[i][j] = atoi(token);
            token = strtok(NULL, "\t");
            j++;
        }
    }
    
    return processes;
    
}