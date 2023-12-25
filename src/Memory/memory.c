#include <stdio.h>
#include <stdlib.h>
#include "memory.h"

Block *initMemory () {
    Block *memory = (Block *) malloc(sizeof(Block));
    memory->size = MAX_SIZE;
    memory->start = 0;
    memory->end = MAX_SIZE - 1;
    memory->next = NULL;
    memory->prev = NULL;
    memory->allocated = 0;
    return memory;
}

Block *findSmallestAvailableBlock (Block *memory, int size) {
    Block *currBlock = memory;
    Block *smallestBlock = NULL;
    while (currBlock != NULL) {
        if (currBlock->allocated == 0 && currBlock->size >= size) {
            if (smallestBlock == NULL)
                smallestBlock = currBlock;
            else if (currBlock->size < smallestBlock->size)
                smallestBlock = currBlock;
        }
        currBlock = currBlock->next;
    }
    return smallestBlock;
}

void splitBlock (Block *block) {
    block->size /= 2;
    block->end = block->start + block->size - 1;
    
    Block *newBlock = (Block *) malloc(sizeof(Block));
    newBlock->size = block->size;
    newBlock->start = block->end + 1;
    newBlock->end = newBlock->start + newBlock->size - 1;
    newBlock->next = block->next;
    newBlock->prev = block;
    newBlock->allocated = 0;
    
    block->next = newBlock;
    if (newBlock->next != NULL)
        newBlock->next->prev = newBlock;
}

// loops through the memory blocks and merges adjacent free blocks
void mergeBlock (Block *block) {
    Block *currBlock = block;
    while (currBlock != NULL) {
        if (currBlock->allocated == 0 && currBlock->next != NULL && currBlock->next->allocated == 0 &&
            currBlock->size == currBlock->next->size) {
            Block *temp = currBlock->next;
            currBlock->size *= 2;
            currBlock->end = temp->end;
            currBlock->next = temp->next;
            if (currBlock->next != NULL)
                currBlock->next->prev = currBlock;
            free(temp);
        } else if (currBlock->next != NULL && currBlock->size == currBlock->next->size)
            currBlock = currBlock->next->next;
        else
            currBlock = currBlock->next;
    }
}

Block *allocateMemory (Block *memory, int size) {
    Block *block = findSmallestAvailableBlock(memory, size);
    if (block == NULL)
        return NULL;
    if (block->size / 2 < size) {
        block->allocated = 1;
        return block;
    }
    splitBlock(block);
    return allocateMemory(block, size);
}

void deallocateMemory (Block *memory, Block *block) {
    block->allocated = 0;
    mergeBlock(memory);
}

//int main() {
//    Block *memory = initMemory();
//    allocateMemory(memory, 100);
//    allocateMemory(memory, 100);
//    allocateMemory(memory, 100);
//    allocateMemory(memory, 100);
//    allocateMemory(memory, 100);
//    deallocateMemory(memory, memory->next->next);
//    deallocateMemory(memory, memory->next->next->next);
//}