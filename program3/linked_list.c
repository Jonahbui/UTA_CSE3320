/* Name: Jonah bui
*  ID: 1001541383
*  linked_list.c
*  Description: implements a linked list
*/
#include "linked_list.h"

#include <stdio.h>
#include <stdlib.h>

List* Create_List()
{
    List* list = malloc(sizeof(List));
    list->head = NULL;
    
    return list;
}

List_Node* Create_List_Node(int num)
{
    List_Node* node = malloc(sizeof(List_Node));
    node->number = num;
    node->next = NULL;
    return node;
}

void Append(List* list, int num)
{
    // New node to append to the end of the list
    List_Node* new_next = Create_List_Node(num);
    
    // Attach new node as head if it is empty
    if(list->head == NULL)
    {
        list->head = new_next;
        return;
    }
    
    // Search to the end of the list and attach the new node
    List_Node* temp = list->head;
    while(temp->next != NULL)
    {
        temp = temp->next;
    }
    temp->next = new_next;
    return;
}

int Remove(List* list, int num, int index)
{
    List_Node* temp = list->head;
    int i;
    for(i = 0; i < index; i++)
    {
        temp = temp->next;
        // Return an error, indexing list node out of bounds
        if(temp == NULL)
        {
            return -1;
        }
    }
} 

void List_View(List* list)
{
    int i = 0;
    List_Node* temp = list->head;
    while(temp != NULL)
    {
        printf("[%d]:%2d\n", i, temp->number);
        temp = temp->next;
        i++;
    }
}
void Clear_List(List* list)
{
    // Seek to the end of list and free each node until NULL reached
    while(list->head != NULL)
    {
        List_Node* temp = list->head;
        list->head = list->head->next;
        free(temp);
    }
}

