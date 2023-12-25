#ifndef OS_SCHEDULER_MEMORY_H
#define OS_SCHEDULER_MEMORY_H

#define MAX_SIZE 1024

typedef struct Block {
    int size;
    int start;
    int end;
    int allocated;
    struct Block *next;
    struct Block *prev;
} Block;

Block *initMemory ();

Block *findSmallestAvailableBlock (Block *memory, int size);

void splitBlock (Block *block);

void mergeBlock (Block *block);

Block *allocateMemory (Block *memory, int size);

void deallocateMemory (Block *memory, Block *block);

#endif //OS_SCHEDULER_MEMORY_H
