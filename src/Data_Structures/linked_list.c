#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"

Node *CreateNode(void *obj)
{
    Node *node = (Node *)malloc(sizeof(Node));
    node->data = obj;
    node->nxt = NULL;
    node->prev = NULL;
    return node;
}

void AddNodeToBack(LinkedList *listPtr, Node *nodePtr)
{
    listPtr->size++;
    if (listPtr->head == NULL)
    {
        listPtr->head = nodePtr;
        listPtr->tail = nodePtr;
    }
    else
    {
        listPtr->tail->nxt = nodePtr;
        nodePtr->prev = listPtr->tail;
        listPtr->tail = nodePtr;
    }
}

void AddNodeToFront(LinkedList *listPtr, Node *nodePtr)
{
    listPtr->size++;
    if (listPtr->head == NULL)
    {
        listPtr->head = nodePtr;
        listPtr->tail = nodePtr;
    }
    else
    {
        listPtr->head->prev = nodePtr;
        nodePtr->nxt = listPtr->head;
        listPtr->head = nodePtr;
    }
}

LinkedList *CreateLinkedList()
{
    LinkedList *list = (LinkedList *)malloc(sizeof(LinkedList));
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return list;
}

void *RemoveNodeFromFront(LinkedList *listPtr)
{
    listPtr->size--;
//    Node *currNode = listPtr->head;
    if (listPtr->head == NULL)
    {
        return NULL;
    }
    else if (listPtr->head == listPtr->tail)
    {
        listPtr->head = NULL;
        listPtr->tail = NULL;
    }
    else
    {
        Node *temp = listPtr->head;
        listPtr->head = listPtr->head->nxt;
        listPtr->head->prev = NULL;
    }
//    return currNode;
}

void *RemoveNodeFromBack(LinkedList *listPtr)
{
//    Node *currNode = tail;
    listPtr->size--;
    if (listPtr->head == NULL)
    {
        return NULL;
    }
    else if (listPtr->head == listPtr->tail)
    {
        listPtr->head = NULL;
        listPtr->tail = NULL;
    }
    else
    {
        Node *temp = listPtr->tail;
        listPtr->tail = listPtr->tail->prev;
        listPtr->tail->nxt = NULL;
    }
//    return currNode;
}

void *RemoveNode(LinkedList *listPtr, Node *nodePtr)
{
    listPtr->size--;
    if (nodePtr == listPtr->head)
    {
        return RemoveNodeFromFront(listPtr);
    }
    else if (nodePtr == listPtr->tail)
    {
        return RemoveNodeFromBack(listPtr);
    }
    else
    {
        nodePtr->prev->nxt = nodePtr->nxt;
        nodePtr->nxt->prev = nodePtr->prev;
    }
//    return nodePtr;
}

void *Peek(LinkedList *listPtr) 
{
    if (listPtr == NULL || listPtr->head == NULL) {
        return NULL;
    }
    return listPtr->head->data;
}