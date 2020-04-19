/* Name: Jonah bui
*  ID: 1001541383
*  page_fault_algorithms.c
*  Description: implements page fault algorithms
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "queue.h"
#include "linked_list.h"
#include "page_fault_algorithms.h"

#define EMPTY -9999999
#define MAX 9999999
// Comments for functions in header file

//-----------------------------------Page Fault Algorithms---------------------------------------//
int FIFO(Queue* queue, int page_table_size)
{
    // Number of faults that occured
    int faults = 0;
    Page page_table[page_table_size];
    // Use to track first-in pages
    Queue* first_used = Create_Queue();
    
    Initialize_Page_Table(page_table, page_table_size);
    
    // Read each number in the queue and determine the number of faults until empty
    while(queue->tail != NULL)
    {
        // Check if the next page in queue is in memory 
        int next = Dequeue(queue);
        int page_found = Reference_Page(page_table, page_table_size, next);
        
        if(!page_found)
        {
            faults+=1;
            // Find an empty entry in memory to put page in if one exists
            int is_empty = Reference_Free_Entry(page_table, page_table_size, next);
            
            // Add fault page to a queue to keep track of the pages first in
            Enqueue(first_used, next);
            // Find a page to evict in order to add new page
            if(!is_empty)
                Evict_FIFO(page_table, page_table_size, next, first_used);
        }
    }
    // Free allocated queue
    Clear_Queue(first_used);
    free(first_used);
    
    return faults;
}

int Optimal(List* list, int page_table_size)
{
    // Number of faults that occured
    int faults = 0;
    Page page_table[page_table_size];
    // Used to track the current page to be refernced
    List_Node* current = list->head;
    // Sets values to avoid junk memory
    Initialize_Page_Table(page_table, page_table_size);
    
    while(current != NULL)
    {
        // Check if page is in memory
        int found = Reference_Page(page_table, page_table_size, current->number);
        if(!found)
        {
            faults+=1;
            // Find an empty entry in memory to put page in if one exists
            int is_empty = Reference_Free_Entry(page_table, page_table_size, current->number);
            
            // Find a page to evict in order to add new page
            if(!is_empty)
                Evict_Optimal(page_table, page_table_size, current->number, current);
        }
        // Go to the next page to reference
        current = current->next;
    }
    return faults;
}

int MFU(List* list, int page_table_size)
{
    // Number of faults that occured
    int faults = 0;
    Page page_table[page_table_size];
    // Used to track the current page to be refernced
    List_Node* current = list->head;
    // Sets values to avoid junk memory
    Initialize_Page_Table(page_table, page_table_size);
    
    while(current != NULL)
    {
        // Check if page is in memory
        int found = Reference_Page(page_table, page_table_size, current->number);
        if(!found)
        {
            faults+=1;
            // Find an empty entry in memory to put page in if one exists
            int is_empty = Reference_Free_Entry(page_table, page_table_size, current->number);
            
            // Find a page to evict in order to add new page
            if(!is_empty)
                Evict_MFU(page_table, page_table_size, current->number);
        }
        // Go to the next page to reference
        current = current->next;
    }
    return faults;
}

int LRU(List* list, int page_table_size)
{
    // Number of faults that occured
    int faults = 0;
    Page page_table[page_table_size];
    // Used to track the current page to be refernced
    List_Node* current = list->head;
    // Sets values to avoid junk memory
    Initialize_Page_Table(page_table, page_table_size);
    
    while(current != NULL)
    {
        // Check if page is in memory
        int found = Reference_Page(page_table, page_table_size, current->number);
        if(!found)
        {
            faults+=1;
            // Find an empty entry in memory to put page in if one exists
            int is_empty = Reference_Free_Entry(page_table, page_table_size, current->number);
            
            // Find a page to evict in order to add new page
            if(!is_empty)
                Evict_LRU(page_table, page_table_size, current->number);
        }
        // Shift the recently used bits to an integer
        Shift_Page_Table(page_table, page_table_size);
        
        // Go to the next page to reference
        current = current->next;
    }
    return faults;
}

//--------------------------Helper Functions--------------------------//
void Initialize_Page_Table(Page page_table[], int page_table_size)
{
    int i;
    for(i = 0; i < page_table_size; i++)
    {
        page_table[i].page_number = EMPTY;
        page_table[i].age = 0;
        page_table[i].recently_used = 0;
        page_table[i].count = 0;
    }
    return;
}

int Reference_Page(Page page_table[], int page_table_size, int ref_page)
{
    int success = 0;
    int i;
    for(i = 0; i < page_table_size; i++)
    {
        if(page_table[i].page_number == ref_page)
        {
            // Page found
            page_table[i].recently_used = 1;
            page_table[i].count+=1;
            success = 1;
            break;
        }
    }
    return success;
}

int Reference_Free_Entry(Page page_table[], int page_table_size, int ref_page)
{
    int success = 0;
    int i;
    for(i = 0; i < page_table_size; i++)
    {
        if(page_table[i].page_number == EMPTY)
        {
            page_table[i].page_number = ref_page;
            page_table[i].age = 0;
            page_table[i].recently_used = 1;
            page_table[i].count = 1;
            success = 1;
            break;
        }
    }
    return success;
}

void Evict_FIFO(Page page_table[], int page_table_size, int ref_page, Queue* first_in)
{
    // Get the number that was first-in to evict
    int first = Dequeue(first_in);
    // Adding set is incorrect since they're already in memory
    int i;
    for(i = 0; i < page_table_size; i++)
    {
        if(page_table[i].page_number == first)
        {
            page_table[i].page_number = ref_page;
            break;
        }
    }
    return;
}

void Evict_LRU(Page page_table[], int page_table_size, int ref_page)
{
    // Start off with first page age to provide a base minimum for all ages to check
    uint32_t lru_page = page_table[0].age;
    int index = 0;
    
    // Find the page with the lowest age to evict and insert new page in memory
    int i;
    for(i = 1; i < page_table_size; i++)
    {
        if(page_table[i].age < lru_page)
        {
            lru_page = page_table[i].age;
            index = i;
        }
    }
    // Put page in memory and make it recently used, so it doesn't get evicted
    page_table[index].page_number = ref_page;
    page_table[index].age = 0;
    page_table[index].recently_used = 1;
    return;
}

void Evict_MFU(Page page_table[], int page_table_size, int ref_page)
{
    int mfu = page_table[0].count;
    int index = 0;
    int i;
    // Get the page with the highest count to replace
    for(i = 1; i < page_table_size; i++)
    {
        if(page_table[i].count > mfu)
        {
            mfu = page_table[i].count;
            index = i;
        }
    }
    // Put page in memory and set the count usage of the new page to 1
    page_table[index].page_number = ref_page;
    page_table[index].count = 1;
    return;
}

void Evict_Optimal(Page page_table[], int page_table_size, int ref_page, List_Node* current_page)
{
    // Used to seek a page in the reference string and determine how long it will take
    // Uses next because we don't want to count the page we are already on
    List_Node* tracker;

    // Tracks length of time of least used page
    int max_length = 0;
    // Tracks how long till page is referenced again
    int length = 0;
    // Tracks index of least used page
    int index = -1;
    
    int last_node = 0;
    
    // Find page that will not be used for the longest period of time
    int i;
    for(i = 0; i < page_table_size; i++)
    {
        // Reset length for each page
        length = 0;
        // Reset pointer to current index 
        tracker = current_page->next;
        // If its the last page in the reference string, then evict any page 
        if(tracker == NULL)
        {
            last_node = 1;
            break;
        }
        
        // Used to count the length of each page
        // Determines which page to evict
        while(page_table[i].page_number != tracker->number)
        {
            length++;
            // If the page is not referenced anymore, replace that page
            if(tracker->next == NULL)
            {
                // In the case that multiple pages will longer be referenced, the last page in the
                // page table that is not referenced will be evicted
                max_length = MAX;
                index = i;
                break;
            }
            tracker = tracker->next;
        }
        
        // If a page has a longer length of time until use, set it as the page to evict
        if(length > max_length)
        {
            index = i;
            max_length = length;
        }
    }
    // Evict the first page if last page in reference string
    if(last_node)
        index = 0;
    
    page_table[index].page_number = ref_page;
    return;
}

void Shift_Page_Table(Page page_table[], int page_table_size)
{
    int i;
    for(i = 0; i < page_table_size; i++)
    {
        // Shift age right to signify page hasn't been used recently
        // Smaller ages means page hasn't been referenced recently
        page_table[i].age = page_table[i].age>>1;
        // If page has been recently used add a 1 to the MSB to signify it's been used recently
        if(page_table[i].recently_used)
            page_table[i].age+=0x80000000;
        
        // Reset recently used bit for the next cycle to determine if page is recently used
        page_table[i].recently_used = 0;
    }
    return;
}

