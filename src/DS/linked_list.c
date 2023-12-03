#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"

Node *CreateNode (void *obj) {
    Node *node = (Node *) malloc(sizeof(Node));
    node->data = obj;
    node->nxt = NULL;
    node->prev = NULL;
    return node;
}

//void AddNode (LinkedList *listPtr, Node *nodePtr) {
//    if (listPtr->head == NULL) {
//        listPtr->head = nodePtr;
//        listPtr->tail = nodePtr;
//    } else {
//        Node *temp = listPtr->head;
//        while (temp != NULL) {
//            if (temp->data == nodePtr->data) {
//                return;
//            }
//            temp = temp->nxt;
//        }
//        listPtr->tail->nxt = nodePtr;
//        nodePtr->prev = listPtr->tail;
//        listPtr->tail = nodePtr;
//    }
//}

void AddNodeToBack (LinkedList *listPtr, Node *nodePtr) {
    if (listPtr->head == NULL) {
        listPtr->head = nodePtr;
        listPtr->tail = nodePtr;
    } else {
        listPtr->tail->nxt = nodePtr;
        nodePtr->prev = listPtr->tail;
        listPtr->tail = nodePtr;
    }
}

void AddNodeToFront (LinkedList *listPtr, Node *nodePtr) {
    if (listPtr->head == NULL) {
        listPtr->head = nodePtr;
        listPtr->tail = nodePtr;
    } else {
        listPtr->head->prev = nodePtr;
        nodePtr->nxt = listPtr->head;
        listPtr->head = nodePtr;
    }
}

LinkedList *CreateLinkedList () {
    LinkedList *list = (LinkedList *) malloc(sizeof(LinkedList));
    list->head = NULL;
    list->tail = NULL;
    return list;
}

void RemoveNodeFromFront (LinkedList *listPtr) {
    if (listPtr->head == NULL) {
        return;
    } else if (listPtr->head == listPtr->tail) {
        free(listPtr->head);
        listPtr->head = NULL;
        listPtr->tail = NULL;
    } else {
        Node *temp = listPtr->head;
        listPtr->head = listPtr->head->nxt;
        listPtr->head->prev = NULL;
        free(temp);
    }
}

void RemoveNodeFromBack (LinkedList *listPtr) {
    if (listPtr->head == NULL) {
        return;
    } else if (listPtr->head == listPtr->tail) {
        free(listPtr->head);
        listPtr->head = NULL;
        listPtr->tail = NULL;
    } else {
        Node *temp = listPtr->tail;
        listPtr->tail = listPtr->tail->prev;
        listPtr->tail->nxt = NULL;
        free(temp);
    }
}

void RemoveNode (LinkedList *listPtr, Node *nodePtr) {
    if (nodePtr == listPtr->head) {
        RemoveNodeFromFront(listPtr);
    } else if (nodePtr == listPtr->tail) {
        RemoveNodeFromBack(listPtr);
    } else {
        nodePtr->prev->nxt = nodePtr->nxt;
        nodePtr->nxt->prev = nodePtr->prev;
        free(nodePtr);
    }
}




