#ifndef MY2LIST_H
#define MY2LIST_H

/* Function prototypes */
struct nodeStruct {
  int item;
  struct nodeStruct *next;
  struct nodeStruct *previous;
};

struct nodeStruct* List_createNode(int item);
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node);
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node);
int List_countNodes (struct nodeStruct *head);
struct nodeStruct* List_findNode(struct nodeStruct *head, int item);
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node);
void List_sort (struct nodeStruct **headRef);

#endif