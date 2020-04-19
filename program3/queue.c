/* Name: Jonah bui
*  ID: 1001541383
*  queue.c
*  Description: implements a queue
*/
#include <stdio.h>
#include <stdlib.h>
#include "queue.h"
#include "linked_list.h"

#define queue_failed -9999999

Queue* Create_Queue()
{
    Queue* queue = malloc(sizeof(Queue));

    // Set to NULL to prevent accessing memory addresses
    queue->head = NULL;
    queue->tail = NULL;
    return queue;
}

Queue_Node* Create_Queue_Node(int num)
{
    Queue_Node* node = malloc(sizeof(Queue_Node));
    // Set the value of the node and make its pointer NULL to prevent accessing memory addresses
    node->number = num;
    node->next = NULL;
    return node;
}

void Enqueue(Queue* q_head, int num)
{
    // Create a node to enqueue to the queue
    Queue_Node* node = Create_Queue_Node(num);
    
    // If there are no nodes currently then that means the head and tails are the same
    if(q_head->tail == NULL)
    {
        q_head->head = node;
        q_head->tail = node;
    }
    // Attach new node to the end of queue and set new tail to newly added node
    else
    {
        q_head->tail->next = node;
        q_head->tail = node;
    }   
}

int Dequeue(Queue* q_head)
{
    // Return an error if user attempts to dequeue empty array
    if(q_head->tail == NULL)
        return queue_failed;
    
    // Store the dequeued node to free malloc
    Queue_Node* temp = q_head->head;
    // Store value of dequeued node to return value
    int num = temp->number;
    
    // If only one node remains, set head and tail to NULL 
    if(q_head->head == q_head->tail)
    {
        q_head->head = NULL;
        q_head->tail = NULL;
        free(temp);
        return num;
    }
    
    // Move the head to the next node
    q_head->head = q_head->head->next;
    free(temp);
    return num;
}

void Clear_Queue(Queue* queue)
{
    while(queue->head != NULL)
    {
        Queue_Node* temp = queue->head;
        queue->head = queue->head->next;
        free(temp);
    }
    return;
}

