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

void mergeBlock (Block *block) {
    int l = 0;
    int r = MAX_SIZE;
    int mid = (l + r) / 2;
    while (1) {
        if (block->start == l && block->end == r - 1)
            return;
        if (block->start < mid) {
            r = mid;
            mid = (l + r) / 2;
            
            if (block->start == l && block->end == r - 1 && block->next != NULL && block->next->allocated == 0 &&
                block->size == block->next->size) {
                block->size *= 2;
                block->end = block->start + block->size - 1;
                block->next = block->next->next;
                if (block->next != NULL)
                    block->next->prev = block;
                mergeBlock(block);
                break;
            }
            
        } else {
            l = mid;
            mid = (l + r) / 2;
            
            if (block->start == l && block->end == r - 1 && block->prev != NULL && block->prev->allocated == 0 &&
                block->size == block->prev->size) {
                block->prev->size *= 2;
                block->prev->end = block->prev->start + block->prev->size - 1;
                block->prev->next = block->next;
                if (block->next != NULL)
                    block->next->prev = block->prev;
                mergeBlock(block->prev);
                break;
            }
        }
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

void deallocateMemory (Block *block) {
    block->allocated = 0;
    mergeBlock(block);
}

//int main () {
//    Block *memory = initMemory();
//    Block *process1 = allocateMemory(memory, 256);
//    Block *process2 = allocateMemory(memory, 92);
//    Block *process3 = allocateMemory(memory, 22);
//    Block *process4 = allocateMemory(memory, 57);
//    Block *process5 = allocateMemory(memory, 48);
//    Block *process6 = allocateMemory(memory, 172);
//    Block *process7 = allocateMemory(memory, 98);
//    deallocateMemory(process6);
//    Block *process8 = allocateMemory(memory, 228);
//    deallocateMemory(process4);
//    deallocateMemory(process8);
//    Block *process9 = allocateMemory(memory, 254);
//    deallocateMemory(process1);
//    Block *process10 = allocateMemory(memory, 233);
//    Block *process17 = allocateMemory(memory, 1);
//    Block *process18 = allocateMemory(memory, 46);
//    Block *process19 = allocateMemory(memory, 21);
//    deallocateMemory(process18);
//    deallocateMemory(process3);
//    deallocateMemory(process5);
//    deallocateMemory(process10);
//    Block *process11 = allocateMemory(memory, 126);
//    deallocateMemory(process11);
//    Block *process12 = allocateMemory(memory, 188);
//    deallocateMemory(process2);
//    deallocateMemory(process7);
//    deallocateMemory(process17);
//}