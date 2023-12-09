#include "headers.h"

/* Modify this file as needed*/
int remainingtime;

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
    return msgq_id;
}

void intializeProcessRemainingTime()
{
    int queueId = intializeMsgQueue(KEY_FILE, MSG_QUEUE_SHCEDULAR_PROCESS_KEY);
    int received = 0;
    messageBuff msg;
    int rec_val = msgrcv(queueId, &msg, sizeof(msg.mIntegerData), getpid(), !IPC_NOWAIT);

    if (rec_val == -1)
    {
        perror("Error in receive");
    }
    remainingtime = msg.mIntegerData;
    printf("Process %d Remaining time = %d\n", getpid(), remainingtime);
}

void initProcess()
{
    initClk();
    intializeProcessRemainingTime();
}

int main(int agrc, char *argv[])
{
    initProcess();
    printf("Hello from process\n");
    // TODO it needs to get the remaining time from somewhere
    // remainingtime = ??;
    int lstTime = getClk();
    while (remainingtime > 0)
    {
        // remainingtime = ??;
        int currClk = getClk();
        if (currClk > lstTime)
        {
            remainingtime--;
            lstTime = currClk;
            printf("Process %d Remaining time = %d\n", getpid(), remainingtime);
        }
    }

    destroyClk(false);

    return 0;
}
