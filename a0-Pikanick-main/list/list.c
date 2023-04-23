#include "list.h"
#include <stdio.h>
#include <stdlib.h>

struct nodeStruct *head = NULL;
struct nodeStruct *current = NULL;
/*
 * Allocate memory for a node of type struct nodeStruct and initialize
 * it with the value item. Return a pointer to the new node.
 */
struct nodeStruct* List_createNode(int item)
{
   struct nodeStruct *node = (struct nodeStruct*) malloc(sizeof(struct nodeStruct));
   node->item=item;
   node->next=NULL; 
   return node;
}

/*
 * Insert node at the head of the list.
 */
void List_insertHead (struct nodeStruct **headRef, struct nodeStruct *node)
{
     node->next=*headRef;
     *headRef=node;
}

/*
 * Insert node after the tail of the list.
 */
void List_insertTail (struct nodeStruct **headRef, struct nodeStruct *node)
{
   if(*headRef == NULL) {
      *headRef = node;
   }
   else
   {
      struct nodeStruct* current = *headRef; // Start at beginning of list
      while(current->next != NULL) 
      {
         
            current = current->next;
      
      }     
      current->next = node; 
   }
}

/*
 * Count number of nodes in the list.
 * Return 0 if the list is empty, i.e., head == NULL
 */
int List_countNodes (struct nodeStruct *head)
{
   int length = 0;
   struct nodeStruct *current;
	current = head;
   while(current != NULL) 
   {
      length++;
      current = current->next;
   }
	
   return length;
}

/*
 * Return the first node holding the value item, return NULL if none found
 */
struct nodeStruct* List_findNode(struct nodeStruct *head, int item)
{
   struct nodeStruct* current = head; // Start at beginning of list

   if(head == NULL) {
      return NULL;
   }

   while(current->item != item) 
   {
      if(current->next == NULL) // last node
      {
         return NULL;
      } 
      else 
      {
         current = current->next;
      }
   }      
   return current;//Match
}

/*
 * Delete node from the list and free memory allocated to it.
 * This function assumes that node has been properly set to a valid node 
 * in the list. For example, the client code may have found it by calling 
 * List_findNode(). If the list contains only one node, the head of the list 
 * should be set to NULL.
 */
void List_deleteNode (struct nodeStruct **headRef, struct nodeStruct *node)
{
   //start from the first link
   struct nodeStruct* current = *headRef;
   struct nodeStruct* previous = NULL;
   int key = node->item;

   while(current->item != key) 
   {
      //for tail
      if(current->next == NULL) 
      {
         return;
      } 
      else 
      {
         //store reference to previous node
         previous = current;
         current = current->next;
      }
   }

   //Match found; reconnect links
   if(current == *headRef) 
   {
      *headRef = (*headRef)->next;
   } 
   else 
   {
      previous->next = current->next;
   }    
   free(node);
}

/*
 * Sort the list in ascending order based on the item field.
 * Any sorting algorithm is fine.
 */
void List_sort (struct nodeStruct **headRef)
{
    int swap;
    struct nodeStruct *a; 
    struct nodeStruct *b = NULL; 

    if (*headRef == NULL) 
    {
        return; 
    }
    do
    { 
        swap = 0; 
        a = *headRef; 
        while (a->next != b) 
        { 
            if (a->item > a->next->item) 
            { 
                int temp = a->item; 
                a->item = a->next->item; 
                a->next->item = temp; 
                swap = 1; 
            } 
            a = a->next; 
        } 
        b = a; 
    } 
    while (swap); 
}