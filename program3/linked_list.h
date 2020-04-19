/* Name: Jonah bui
*  ID: 1001541383
*  linked_list.h
*  Description: interface for linked list
*/
#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include <stdio.h>
#include <stdlib.h>

typedef struct linked_list_node
{
	int number;
	struct linked_list_node* next;
}List_Node;

typedef struct linked_list
{
	struct linked_list_node* head;
}List;

/*
Description: creates a linked list with no nodes.
Parameters: nothing
Returns: a pointer to a linked list
*/
List* Create_List();

/*
Description: creates a node and initializes it values.
Parameters:
> num: an integer number to store in the node
Returns: a pointer to a node
*/
List_Node* Create_List_Node(int num);

/*
Description: adds a node to the end of a linked list
Parameters:
> list: a pointer to the linked list that holds the nodes
> num: an integer number to store in the node.
Returns: nothing
*/
void Append(List* list, int num);

/*
Description: removes a node from a provided index
Parameters:
> list: a pointer to the linked list that holds the nodes
> num: an integer number to store in the node.
> index: the index number of the node to remove (determined by position in list).
Returns:
*/
int Remove(List* list, int num, int index);

/*
Description: prints out a visual representation of the linked list.
Parameters:
> list: a pointer to the linked list that holds the nodes to be printed
Returns: nothing
*/
void List_View(List* list);

/*
Description: deletes all the nodes in a linked list and frees them.
Parameters:
> list: a pointer to the linked list that holds the nodes to be freed
Returns: nothing
*/
void Clear_List(List* list);
#endif

