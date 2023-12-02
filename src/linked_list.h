#ifndef LINKED_LIST
#define LINKED_LIST

struct node;
typedef struct node
{
  node *nxt;
  node *prev;
  void *data;
} Node;

/**
 * @brief creates a linked list node
 * @param obj the object to add reference to in the linked list
 * @return pointer to linked lisr node created
 */
Node *AddNode(void *obj);

/**
 * @brief adds node to end of linked list
 * @param  nodePtr pointer to node
 * @return void
 */
void AddNodeToBack(Node *nodePtr);

/**
 * @brief adds node to front of a linked list
 * @param nodePtr pointer to node
 * @return void
 */
void AddNodeToFront(Node *nodePtr);

#endif