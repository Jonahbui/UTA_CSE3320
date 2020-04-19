/* Name: Jonah bui
*  ID: 1001541383
*  pf.c
*  Description: a program used to learn more about page faulting
*  by implementing different page fault algorithms.
*/

// The MIT License (MIT)
//
// Copyright (c) 2020 Trevor Bakker
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "queue.h"
#include "page_fault_algorithms.h"

#define MAX_LINE 1024

int main( int argc, char * argv[] ) 
{   
    // Create queue for FIFO algorithm
    Queue* queue = Create_Queue();
    List* list = Create_List();

    char * line = NULL;
    size_t line_length = MAX_LINE;
    char * filename;

    FILE * file;

    if( argc < 2 )
    {
        printf("Error: You must provide a datafile as an argument.\n");
        printf("Example: ./fp datafile.txt\n");
        exit( EXIT_FAILURE );
    }

    filename = ( char * ) malloc( strlen( argv[1] ) + 1 );
    line     = ( char * ) malloc( MAX_LINE );

    memset( filename, 0, strlen( argv[1] + 1 ) );
    strncpy( filename, argv[1], strlen( argv[1] ) );

    printf("Opening file %s\n", filename );
    file = fopen( filename , "r");

    if ( file ) 
    {
        while ( fgets( line, line_length, file ) )
        {
          char * token;

          token = strtok( line, " ");
          int working_set_size = atoi( token );

          printf("\nWorking set size: %d\n", working_set_size );
     
          while( token != NULL )
          {
            token = strtok( NULL, " " );
            
            if( token != NULL )
            {
                //printf("Request: %d\n", atoi( token ) );
                
                // Add numbers to queue for processing in the algorithms
                Enqueue(queue, atoi(token));
                Append(list, atoi(token));
            }
          }
          printf("\n");
          // Track total amount of faults from each pf algorithm
          int faults_FIFO = FIFO(queue, working_set_size);
          int faults_LRU = LRU(list, working_set_size);
          int faults_MFU = MFU(list, working_set_size);
          int faults_optimal = Optimal(list, working_set_size);
          
          Clear_List(list);
          Clear_Queue(queue);
          // Print results of page fault algorithms
          printf("%-25s%2d\n","Page faults of FIFO:", faults_FIFO);
          printf("%-25s%2d\n","Page faults of LRU:", faults_LRU);
          printf("%-25s%2d\n","Page faults of MFU:", faults_MFU);
          printf("%-25s%2d\n","Page faults of Optimal:", faults_optimal);
        }

        
        // Free mallocs to avoid memory leaks
        free(queue);
        free(list);
        free( line );
        fclose(file);
        free(filename);
    }
  return 0;
}

