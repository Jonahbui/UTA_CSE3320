/* Author: Jonah Bui
 * ID: 1001541383
 * Parnter: Burhanuddin Chinwala
 * ID: 1001578263
 * Purpose: make a program to read in a fat32 filesystem.
 */

/* To do list:
 * - Need to fix memory leak due to continues in all functions.
 * Cannot continue or else free will not work.
 * - May need to fix find_file since it might get things other than files.
 * - Need to fix file not found
*/

// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
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

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include "compare.h"
#include "queue.h"

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
                                // so we need to define what delimits our tokens.
                                // In this case  white space
                                // will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 5     // Mav shell only supports five arguments

typedef struct file_system_info
{
  uint16_t BPB_BytesPerSec;
  uint8_t BPB_SecPerClus;
  uint16_t BPB_RsvdSecCnt;
  uint8_t BPB_NumFATS;
  uint32_t BPB_FATSz32;
}FSI;

struct __attribute__((__packed__)) DirectoryEntry
{
  u_char    Name[11]; // Do I need to specify u_char?
  uint8_t   Attr;
  uint8_t   Unused1[8];
  uint16_t  FirstClusterHigh;
  uint8_t   Unused2[4];
  uint16_t  FirstClusterLow;
  uint32_t  FileSize;
};

// Store and access file and file info
FILE* fp = NULL;
FSI* info;

/*
 * Function     : LBAToOffset
 * Parameters   : The current sector number that points to a block of data
 * Returns:     : The value of the address for that block of data
 * Description  : Finds the starting addresss of a block of data given the sector number
 * corresponding to that data block.
*/
int LBAToOffset(int32_t sector)
{
  return((sector-2)*info->BPB_BytesPerSec) + (info->BPB_BytesPerSec * info->BPB_RsvdSecCnt) 
  + (info->BPB_NumFATS * info->BPB_FATSz32 * info->BPB_BytesPerSec);
}

/* Function: NextLB
 * Parameters: The current sector number that points to a block of data and the 
 * Return: The logical block address of the block in the file
 * Description: Given a logical block address, look up into the first FAT 
 * and return the logical block address of the block in the file. 
 * If there is no further blocks then return -1
*/
int16_t NextLB(uint32_t sector)
{
  uint32_t FATAddress = ( info->BPB_BytesPerSec * info->BPB_RsvdSecCnt) + (sector * 4);
  int16_t val;
  fseek(fp, FATAddress, SEEK_SET);
  fread(&val, 2, 1, fp);
  return val;
}

/* Function: Directory_Info
 * Parameters: A DirectoryEntry array to hold information in a directory, and the address of the
 * directory to pull the files from.
 * Return: Nothing
 * Description: Gets all the files in the directory absolute address passed in. 
*/
void Directory_Info(struct DirectoryEntry dir[16], uint32_t cwd)
{
  //printf("Directory: %x\n\n", cwd);
  fseek(fp,cwd,SEEK_SET);
  fread(dir, 32*16, 1, fp);
  return;
}

/* Function: Print_directory
 * Parameters: A DirectoryEntry that holding exisiting information of a directory.
 * Return: Nothing
 * Description: Given a directory's files, it will print out all the files in it that should be
 * displayed to a user. Does not include deleted files, system volumes, or system files.
*/
void Print_Directory(struct DirectoryEntry dir[16])
{
    int i, j;
    // Search through the directory and print only the files that the user should see
    for(i = 0; i < 16; i++)
    {
      // Do not printed out any files that have been deleted. Don't want users seeing them.
      if(dir[i].Name[0] == 0xe5)
        return;

      // If the file: is read-only file, describes a subdirectory, or is a modified file, then
      // print it out for the user to see.
      // Don't want to show system files or system volume names to the user.
      if(dir[i].Attr == 0x01 || dir[i].Attr == 0x10 || dir[i].Attr == 0x20 || dir[i].Attr == 0x30)
      {
        // If the first char of filename is 0x05, it acutally 0xe5. So replace it and print
        // out the actual filename, then place back 0x05 so it won't be read as a deleted file.
        if(dir[i].Name[0] == 0x05)
        {
          dir[i].Name[0] = 0xe5;
          for(j=0;j<11;j++)
            printf("%c", dir[i].Name[j]);
          dir[i].Name[0] = 0x05;
        }
        else
        {
          for(j=0;j<11;j++)
            printf("%c", dir[i].Name[j]);
        }
        printf("\n");
      }
    }
    printf("\n");
    return;
}

/* Function: Find_File
 * Parameters: A DirectoryEntry that holds existing files in a directory and a filename to match.
 * Return: The index in the DirectoryEntry if file is found; -1 if the file is not found
 * Description: Searches through a DirectoryEntry attempting to find a file that has a matching
 * filename. If the file is not found it will return a -1, else it will return the location of the
 * file.
*/
int Find_File(struct DirectoryEntry dir[16], char* match)
{
  int i;
  for(i = 0; i < 16; i++)
  {
    // Find a matching filename in the directory struct and print its info. If no matching
    // filename is found then warn the user and continue on.
    // Check if command is . or .. since cannot use compare function because it strtoks
    // '.' characters.
    if(strcmp(match, ".") != 0 && strcmp(match,"..") != 0 && compare(dir[i].Name, match))
      return i;
    else if(dir[i].Name[0] == 0x2e && dir[i].Name[1] != 0x2e && strcmp(match,".") == 0)
      return i;
    else if(dir[i].Name[0] == 0x2e && dir[i].Name[1] == 0x2e && strcmp(match,"..") == 0)
      return i;
  }
  return -1;
}

int main()
{
  char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

  // Use to store current directory and current directory info
  uint32_t cwd = 0;
  uint32_t root = 0;

  // Use to store information about the directory files
  struct DirectoryEntry dir[16];
  memset(dir, '\0', 16);

  while( 1 )
  {
    // Print out the mfs prompt
    printf ("mfs> ");

    // Read the command from the commandline.  The
    // maximum command that will be read is MAX_COMMAND_SIZE
    // This while command will wait here until the user
    // inputs something since fgets returns NULL when there
    // is no input
    while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

    if(strcmp(cmd_str,"\n") == 0)
      continue; 

    /* Parse input */
    char *token[MAX_NUM_ARGUMENTS];

    int   token_count = 0;                                 
                                                           
    // Pointer to point to the token
    // parsed by strsep
    char *arg_ptr;                                         
                                                           
    char *working_str  = strdup( cmd_str );                

    // we are going to move the working_str pointer so
    // keep track of its original value so we can deallocate
    // the correct amount at the end
    char *working_root = working_str;

    // Tokenize the input stringswith whitespace used as the delimiter
    while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
              (token_count<MAX_NUM_ARGUMENTS))
    {
      token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
      if( strlen( token[token_count] ) == 0 )
      {
        token[token_count] = NULL;
      }
        token_count++;
    }

    // If the user inputs a command that would cause a NULL string on the first parameter ignore it
    // and warn the user.
    if(token[0] == NULL)
    {
      printf("Error: Invalid command\n\n");
      continue; 
    }

    // Opens a file for the program to read
    if(strcmp(token[0], "open") == 0)
    {
      //Check if file is already open to avoid opening it twice
      if(fp != NULL)
      {
        printf("Error: File system image already open\n");
        continue;
      }

      // Open the file
      fp = fopen(token[1], "r+");
      
      if(fp == NULL)
      {
        printf("Error: File system image not found\n");
        continue;
      }  

      // Allocate struct to store file system inforamtion. Want this data to persist while the file
      // is open. Also want available as soon as the file is opened to work with.
      info = malloc(sizeof(FSI));
      
      // fseek parameters:
      //    Param 1: file to read from
      //    Param 2: set offset
      //    Param 3: position from which offset is added
      // fread paramteres:
      //    Param 1: where to store data
      //    Param 2: number of bytes to read
      //    Param 3: how many times to get bytes specified in param 2
      //    Param 4: where to read bytes from 
      fseek(fp, 11, SEEK_SET);
      fread(&(info->BPB_BytesPerSec), 2, 1, fp);

      fseek(fp, 13, SEEK_SET);
      fread(&(info->BPB_SecPerClus), 1, 1, fp);

      fseek(fp ,14, SEEK_SET);
      fread(&(info->BPB_RsvdSecCnt), 2, 1, fp);
      
      fseek(fp, 16, SEEK_SET);
      fread(&(info->BPB_NumFATS), 1, 1, fp);

      fseek(fp, 36, SEEK_SET);
      fread(&(info->BPB_FATSz32), 4, 1, fp);
      
      // Set current directory to root directory for file system
      cwd = root = LBAToOffset(2);
      Directory_Info(dir, cwd);
    }
    else if(strcmp(token[0], "close") == 0)
    {
      if(fp == NULL)
      {
        printf("Error: File system not open\n\n");
        continue;
      }
      fclose(fp);
      fp = NULL;
      free(info);
      info = NULL;
    }
    else if(strcmp(token[0], "quit") == 0)
    {
      if(fp != NULL)
      {
        fclose(fp);
        free(info);
        fp = NULL;
        info = NULL;
      }
      free(working_root);
      free(working_str);
      free(cmd_str);
      int i;
      for(i = 0; i < token_count; i++)
        free(token[i]);
      exit(0);
    }
    else if(fp)
    {
      if(strcmp(token[0], "info") == 0)// GOOD Break
      {
        printf("\nBPB_BytesPerSec:  \nDec: %4d\n", info->BPB_BytesPerSec);
        printf("Hex: %4x\n\n", info->BPB_BytesPerSec);
        printf("BPB_SecPerClus:  \nDec: %4d\n", info->BPB_SecPerClus);
        printf("Hex: %4x\n\n", info->BPB_SecPerClus);
        printf("BPB_RsvdSecCnt:  \nDec: %4d\n", info->BPB_RsvdSecCnt);
        printf("Hex: %4x\n\n", info->BPB_RsvdSecCnt);
        printf("BPB_NumFATS:     \nDec: %4d\n", info->BPB_NumFATS);
        printf("Hex: %4x\n\n", info->BPB_NumFATS);
        printf("BPB_FATSz32:     \nDec: %4d\n", info->BPB_FATSz32);
        printf("Hex: %4x\n\n", info->BPB_FATSz32);
      }
      else if(strcmp(token[0], "stat") == 0)// GOOD Break
      {
        // To prevent seg fault. Make sure user enters enough args. Need 2 (3 due to NULL string)
        if(token_count < 3)
        {
          printf("Error: stat needs 2 valid arguments\n\n");
        }
        else
        {
          int i = Find_File(dir, token[1]);
          if (i != -1)
          {
            printf("Attributes:\tFile Size:\tStarting Cluster Number:\n");
            printf("%-11d\t%-10d\t%-24d\n",dir[i].Attr, dir[i].FileSize, dir[i].FirstClusterLow);
            printf("%-11x\t%-10x\t%-24x\n\n",dir[i].Attr, dir[i].FileSize, dir[i].FirstClusterLow);
          }
          else
            printf("Error: File not found\n\n");
        }
      }
      else if(strcmp(token[0], "get") == 0)// GOOD Break
      {
        // If not enough parameters entered, warn user and continue on. Don't want to read a NULL
        // string. That will seg fault the program.
        if(token_count < 3 && token[1] == NULL)
        {
          printf("Error: get needs 2 valid arguments\n\n");
        }
        else
        {
          // Don't get a file if it is already in the current working directory. Redundant.
          int i = Find_File(dir, token[1]);
          if(i == -1)
          {
            // Create a queue to do a breadth-first search from the root directory to find the file
            // to place into the current working directory.
            Queue* queue = Create_Queue();

            // Use to check if the file in question has been found. Will be used to stop the 
            // breadth-first search alongside the size of the queue.
            int file_found = -1;
            
            // Enqueue the root directory to start the breadth-first search since initial queue 
            // cannot be empty.
            Enqueue(queue, root);
            
            // Search until the queue is empty (meaning unsucessful search) or until the file is 
            //found.
            while(queue->tail != NULL && file_found == -1)
            {
              // Read in the current dequeued address and check if it has the matching file which
              // would complete the search. Else enqueue all the valid directories to the BFS queue.
              int dequeue_address = Dequeue(queue);
              Directory_Info(dir, dequeue_address);
              file_found = Find_File(dir, token[1]);
              
              // If the file has been found. Add the file info to the current working directory
              // and mark it as deleted
              if(file_found != -1)
              {
                // Use dump to find a free entry in the current working directory by making it 
                // read in each 32 bytes and checking if the filename has either been deleted or
                // has not been used
                struct DirectoryEntry dump;
                
                // Seek to the current working directory to read in the entries until one is free.
                fseek(fp, cwd, SEEK_SET);

                // Finds a free slot to place the file
                int j;
                for(j = 0; j < 16; j++)
                {
                  fread(&dump, 32, 1, fp);

                  // Found a free file slot, write the file there
                  // Free if the entry has a filename not in use or is deleted.
                  if(dump.Name[0] == 0x00 || dump.Name[0] == 0xe5)
                  {
                    fseek(fp, cwd+32*j, SEEK_SET);
                    fwrite(&dir[file_found], 32, 1, fp);

                    // Mark the original file as deleted
                    fseek(fp, dequeue_address+32*file_found, SEEK_SET);
                    u_char delete = 0xe5;
                    fwrite(&delete, 1, 1, fp);
                    break;
                  }
                }
                break;
              }
              else
              {
                // The file could not be found, but queue up all directories in the dequeued 
                // directory to continue searching for the file
                int i;
                for(i = 0; i < 16; i++)
                {
                  //Do not want to enqueue deleted directories. 
                  if(dir[i].Attr == 0x10 && dir[i].Name[0] != 0xe5 && dir[i].Name[0] != 0x00)
                  {
                    // Do not want to enqueue again any links to the dequeued directory or the root 
                    // directory (or the queue will loop indefinitely). 
                    int check_address = dir[i].FirstClusterLow;
                    if(check_address != root && check_address != dequeue_address &&
                    dir[i].Name[0] != 0x2e && dir[i].Name[1] != 0x2e)
                    {
                      Enqueue(queue, LBAToOffset(check_address));
                    }
                  }
                }// For loop
              }
            }// End of while loop
            if(file_found == -1)
            {
              printf("Error: File not found.\n\n");
            }
            Clear_Queue(queue);
            free(queue);
          }
        }
      }
      else if(strcmp(token[0], "cd") == 0)
      {
        // If not enough parameters entered, warn user and continue on. Don't want to read a NULL
        // string. That will seg fault the program.
        if(token_count < 3 && token[1] == NULL)
        {
          printf("Error: cd needs 2 valid arguments\n\n");
        }
        else
        {
          // Parse the input to check if it is an absolute path.
          // If there are two valid strings delimited by / then it is an absolute path typed in.
          char * cd_cmd_str = (char*) malloc(strlen(token[1])*sizeof(char));
          strncpy(cd_cmd_str, token[1], strlen(token[1]));

          /* Parse input */
          char *cd_token[256];

          int cd_count = 0;                                 
                                                                
          // Pointer to point to the token
          // parsed by strsep
          char *cd_arg_ptr;                                         
                                                                
          char *cd_working_str  = strdup( cd_cmd_str );                

          // we are going to move the working_str pointer so
          // keep track of its original value so we can deallocate
          // the correct amount at the end
          char *cd_working_root = cd_working_str;

          // Tokenize the input stringswith whitespace used as the delimiter
          while ( ( (cd_arg_ptr = strsep(&cd_working_str, "/\\" ) ) != NULL) && 
                    (cd_count<MAX_NUM_ARGUMENTS))
          {
            cd_token[cd_count] = strndup( cd_arg_ptr, MAX_COMMAND_SIZE );
            if( strlen( cd_token[cd_count] ) == 0 )
            {
              cd_token[cd_count] = NULL;
            }
            else
              cd_count++;
          }

          // Find the matching filename. If the filename is to a directory then change the current
          // offset to that directory. However, if it is a file, warn the user and continue on. 
          // Don't try to cd into a file or else system will crash
          uint32_t offset;
          int index, token_index;

          for(token_index=0; token_index < cd_count; token_index++)
          {
            index = Find_File(dir, cd_token[token_index]);
            if(index != -1)
            {
              //If match found, check if filename is a directory because we don't want cd into a 
              // a file, it will cause a seg fault.
              if(dir[index].Attr == 0x10)
              {
                if(dir[index].FirstClusterLow == 0x0000)
                  offset = LBAToOffset(2);
                else
                  offset = LBAToOffset(dir[index].FirstClusterLow);
                Directory_Info(dir, offset);
              }
              else
                printf("Error: not a directory\n\n");
            }
            else
            {
              // Can't find the searched directory so it doesn exist
              break;
            }
          }

          // If search is succesful, set cwd to the searched directory
          if(index != -1)
          {
            cwd = offset;
            Directory_Info(dir, cwd);
          }
          else
          {
            // Check if absolute address
            // Check if the root directory to see if starting directory is in root
            Directory_Info(dir, root);

            for(token_index=0; token_index < cd_count; token_index++)
            {
              index = Find_File(dir, cd_token[token_index]);
              if(index != -1)
              {
                //If match found, check if filename is a directory because we don't want cd into a 
                // a file, it will cause a seg fault.
                if(dir[index].Attr == 0x10)
                {
                  if(dir[index].FirstClusterLow == 0x0000)
                    offset = LBAToOffset(2);
                  else
                    offset = LBAToOffset(dir[index].FirstClusterLow);
                  Directory_Info(dir, offset);
                }
                else
                  printf("Error: not a directory\n\n");
              }
              else
              {
                // Can't find the searched directory so it doesn exist
                break;
              }
            }
            // If search is succesful, set cwd to the searched directory
            if(index != -1)
            {
              cwd = offset;
              Directory_Info(dir, cwd);
            }
            else if(strcmp(cd_token[0],"~") == 0)
            {
              cwd = root;
              Directory_Info(dir, cwd);
            }
            else
            {
              //printf("Error: Directory not found\n\n");
              Directory_Info(dir, cwd);
            }
          }
          int t  = 0;
          for( t = 0; t < cd_count; t ++ ) 
            free(cd_token[t]);
          free(cd_working_root);
          free(cd_working_str);
          free(cd_cmd_str);
        }
      }
      else if(strcmp(token[0], "ls") == 0)
      {
        // Check if the command is .. to print parent directory. If parent directory cannot be
        // be found, print out the current directory.
        int index = -1;
        if(token[1] != NULL && strcmp(token[1], "..") == 0)
        {
          // Use to store the parent directory address to cd into. Use this to avoid tampering with
          // the current working directory address and losing it
          int offset;
          index = Find_File(dir, "..");
          if(index != -1)
          {
            // If the cluster lower number is 0x0000 then it is a root directory.
            // So go to root.
            if(dir[index].FirstClusterLow == 0x0000)
              offset = LBAToOffset(2);
            else
              offset = LBAToOffset(dir[index].FirstClusterLow);
            // Go to the parent directory and print out its directory
            Directory_Info(dir, offset);
            Print_Directory(dir);

            // Restore the initial file directory info
            Directory_Info(dir, cwd);
          }
        }
        // If user didn't do "cd .." cwd is already at root, print out the cwd.
        // Current working directory (cwd)
        if(index == -1)
        {
          Directory_Info(dir, cwd);
          Print_Directory(dir);
        }
      }
      else if(strcmp(token[0], "read") == 0)
      {
        // Need a total of 4 arguments (5 because of last NULL string in token)
        if(token_count < 5)
        {
          printf("Error: read needs 4 valid arguments\n\n");
        }
        else
        {
          // Use to follow file clusters and determine where to read data.
          uint32_t offset = 0;

          int index = Find_File(dir, token[1]);
          if(index != -1 && dir[index].Attr == 0x10)
          {
            // Use to store the position to offset from and the number of bytes to read in
            int position = atoi(token[2]);
            int bytes_to_read;
            if(strcmp(token[3], "max") == 0)
              bytes_to_read = dir[index].FileSize;
            else
              bytes_to_read = atoi(token[3]);

            // Use to print out all the characters stored during the read. Can't just print out
            // string or else garbage may also be printed. Must do character at a time.
            int bytes_to_read_temp = bytes_to_read;

            // Must set new file size if there is a position offset into the file, so we will not
            // read the whole file size.
            int num_readable_bytes = dir[index].FileSize - position;

            // Get the cluster address of the file's content to start reading the data in.
            int lb = dir[index].FirstClusterLow;
            offset = LBAToOffset(lb) + position;

            // Set the initial position to read the file from.
            fseek(fp, offset, SEEK_SET);
            
            // Can't read more than the given file size, so set the new number of bytes to read to
            // the number of readable bytes (max file size - byte offset from file beggining).
            if(bytes_to_read_temp > num_readable_bytes)
            {
              bytes_to_read = num_readable_bytes;
              bytes_to_read_temp = num_readable_bytes;
            }
              
            // Store the input of what is read in to display to user.
            u_char bytes_read[bytes_to_read_temp];
            memset(bytes_read, ' ', bytes_to_read_temp);

            // Need to keep track of where to fread into array if the file size exceeds 512 to avoid
            // overwriting existing information
            int arr_pos = 0;

            while(lb != -1)
            {
              // If there is less than 512 bytes left to read,
              if(bytes_to_read_temp < 512)
              {
                fread(&bytes_read[arr_pos], bytes_to_read_temp, 1, fp);
                bytes_to_read_temp = 0;
                break;
              }
              // Read 512 bytes until less than 512 left
              fread(&bytes_read[arr_pos], 512, 1, fp);
              bytes_to_read_temp-=512;
              arr_pos+=512;

              // Find the next cluster to continue reading data from if filesize > 512
              lb = NextLB(lb);

              // If at the end of the logical block, dont try to grab more info
              if(lb != -1)
              {
                offset = LBAToOffset(lb);
                fseek(fp, offset, SEEK_SET);
              }
            }
            
            // Print out the bytes read in to display to user
            int i;
            for(i = 0; i < bytes_to_read; i++)
              printf("%2x ", bytes_read[i]);
            printf("\n");
          }
          else
          {
            printf("Error: file not found or not a file\n\n");
          }
        }
      }
    }
    free( working_root );
    free(working_str);
    int i;
    for(i = 0; i < token_count; i++)
    {
      free(token[i]);
    }
  }
  return 0;
}
