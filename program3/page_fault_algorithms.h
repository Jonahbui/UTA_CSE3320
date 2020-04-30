/* Name: Jonah bui
*  ID: 1001541383
*  page_fault_algorithms.h
*  Description: interface for page fault algorithms
*/
#ifndef PAGE_FAULT_ALGORITHMS_H
#define PAGE_FAULT_ALGORITHMS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "queue.h"
#include "linked_list.h"

typedef struct page
{
	int page_number;
	uint32_t age;
	int recently_used;
	int count;
}Page;

//--------------------------Helper Functions--------------------------//

/*
Description: provides default values for an initialized page table
Parameters:
> page_table[]: an array of pages that have just been declared 
> page_table_size: the size of the working set which is also the size of the page table
Returns: 1 if referenced page is in memory, 0 if not
*/
void Initialize_Page_Table(Page page_table[], int page_table_size);

/*
Description: determines if a page is in the page table and also sets the recently used bit if so
Parameters:
> page_table[]: an initialized array of pages 
> page_table_size: the size of the working set which is also the size of the page table
> ref_page: the current page (represented as an integer) being searched in the page table
Returns: 1 if referenced page is in memory, 0 if not
*/
int Reference_Page(Page page_table[], int page_table_size, int ref_page);

/*
Description: attempts to fill the page table if an empty slot is available with the referring page.
If successful, it will also set the reference bit and the age of the new page inserted in memory.
Parameters:
> page_table[]: an initialized array of pages 
> page_table_size: the size of the working set which is also the size of the page table
> ref_page: the current page (represented as an integer) being searched in the page table
Returns: 1 if page table had a free entry and placed page in memory, 0 if not
*/
int Reference_Free_Entry(Page page_table[], int page_table_size, int ref_page);

/*
Description: using a FIFO algorithm, determines which page to evict and places a new page in the 
page table. It will also set the reference bit and reset the age of the page if a previous page is
evicted.
Parameters:
> page_table[]: an initialized array of pages 
> page_table_size: the size of the working set which is also the size of the page table
> ref_page: the current page (represented as an integer) being searched in the page table
> first_in: a queue with each referenced page in a node
Returns: nothing
*/
void Evict_FIFO(Page page_table[], int page_table_size, int ref_page, Queue* first_in);

/*
Description: using a LRU algorithm, determines which page to evict and places a new page in the 
page table. It will also set the reference bit and reset the age of the page if a previous page is
evicted.
Parameters:
> page_table[]: an initialized array of pages 
> page_table_size: the size of the working set which is also the size of the page table
> ref_page: the current page (represented as an integer) being searched in the page table
Returns: nothing
*/
void Evict_LRU(Page page_table[], int page_table_size, int ref_page);

/*
Description: using an most frequently used algorithm, determines which page to evict and places a 
new page in the page table. It will also set the reference bit and reset the age of the page if a 
previous page is evicted.
Parameters:
> page_table[]: an initialized array of pages 
> page_table_size: the size of the working set which is also the size of the page table
> ref_page: the current page (represented as an integer) being searched in the page table
Returns: nothing
*/
void Evict_MFU(Page page_table[], int page_table_size, int ref_page);
/*
Description: using an Optimal algorithm, determines which page to evict and places a new page in 
the page table. It will also set the reference bit and reset the age of the page if a previous page
is evicted.
Parameters:
> page_table[]: an initialized array of pages 
> page_table_size: the size of the working set which is also the size of the page table
> ref_page: the current page (represented as an integer) being searched in the page table
> current_page: a pointer to a node that contains the current page in a linked list
Returns: nothing
*/
void Evict_Optimal(Page page_table[], int page_table_size, int ref_page, List_Node* current_page);

/*
Description: used to implement aging. Right shifts the age integer of a page table entry and 
resets the recently used bit. If the page was recently used, it "shifts" a 1 into the most
significant bit.
Parameters:
> page_table[]: an initialized array of pages 
> page_table_size: the size of the working set which is also the size of the page table.
Returns: nothing
*/
void Shift_Page_Table(Page page_table[], int page_table_size);

//--------------------------Page Fault Algorithms--------------------------//
/*
Description: implements the first in, first out algorithm on a page reference string given a 
working set size
Parameters:
> queue: a queue that contains each page as a node in the queue
> page_table_size: the size of the working set
Returns: the integer number of faults that have occured given a reference string
*/
int FIFO(Queue* queue, int page_table_size);

/*
Description: implements the least recently used algorithm on a page reference string given a 
working set size
Parameters: 
> list: a list that contains each page referenced in a node
> page_table_size: the size of the working set
Returns: the integer number of faults that have occured given a reference string
*/
int LRU(List* list, int page_table_size);
/*
Description:implements the most frequently used algorithm on a page reference string given a 
working set size
Parameters:
> list: a list that contains each page referenced in a node
> page_table_size: the size of the working set
Returns: the integer number of faults that have occured given a reference string
*/
int MFU(List* list, int page_table_size);

/*
Description: implements the optimal algorithm on a page reference string given a 
working set size
Parameters: 
> list: a list that contains each page referenced in a node
> page_table_size: the size of the working set
Returns: the integer number of faults that have occured given a reference string
*/
int Optimal(List* list, int page_table_size);
#endif

