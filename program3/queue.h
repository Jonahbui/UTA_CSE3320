/* Name: Jonah bui
*  ID: 1001541383
*  queue.h
*  Description: interface for queue
*/
#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct queue_node
{
	int number;
	struct queue_node* next;
}Queue_Node;

typedef struct queue_head
{
	Queue_Node* head;
	Queue_Node* tail;
}Queue;

/*
Description: used to create a queue. It will store all information about the queue.
Parameters: nothing
Returns: a pointer to an empty queue
*/
Queue* Create_Queue();

/* Used to create a node that can be enqueued.
Description: used to create a queue. It will store all information about the queue.
Parameters: 
> num: an integer to store
Returns: a pointer to the node
*/
Queue_Node* Create_Queue_Node(int num);

/* 
Description: used to add a new node to the end of the queue.
Parameters:
> q_head: a pointer to the queue to add a node
> num: integer number to store
Returns: nothing
*/
void Enqueue(Queue* q_head, int num);

/* 
Description: used to remove a node from the front of the queue.
Parameters:
> q_head: a pointer to the queue to remove node
Returns: the integer number stored in the queue node. It return queue_failed (defined in c file)
if it failed.
*/
int Dequeue(Queue* q_head);

/*
Description: clears all nodes in a queue and frees them.
Parameters: 
> queue: a queue that contains nodes to be freed
Returns: none
*/
void Clear_Queue(Queue* queue);
#endif

