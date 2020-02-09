/* Name: Jonah Bui
 * ID: 1001541383
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS_SIZE 10
#define WHITESPACE " \t\n"

int main(int argc, char** argv)
{
    //Use to store user input for commands and for parsing
    char* cmd_str = (char*)malloc(MAX_INPUT_SIZE);
    
    //Use to store the ID of child processes and keep track of how many have spawned
    int pids[15];
    int pid_count = 0;

    //Use to store history so that users may view it and reuse them
    char* history[15];
    int i;
    static int count = 0;
    for(i = 0; i < 15; i++)
    {
        history[i] = (char*)malloc(MAX_INPUT_SIZE);
        memset(history[i], 0, MAX_INPUT_SIZE);
    }

    //Loop the program forever until the user quits/exits
    while(1)
    {
        printf("msh> ");

        //Repeatedly get user command input until exit/quit
        while(fgets(cmd_str, MAX_INPUT_SIZE, stdin) != NULL)
        {
            //Ignore input if it was just ENTER to avoid running fork
            if(strcmp(cmd_str, "\n") == 0)
            {
                break;
            }
            else if(strpbrk(cmd_str, "!1234567890") != NULL)
            {
                //Checks if user is reusing command in history with matching index number
                //There is an attempt to use history if ! is found in the cmd_str
                char* token = strtok(cmd_str, "!");
                int history_num = atoi(token)-1;
                
                //Allow user to reuse previous commands if in range
                //Range: cannot be greater than size of current history if less than 15.
                //Cannot also be greater than 15.
                if(history_num <= count && history_num <= 15)
                {
                    strcpy(cmd_str, history[history_num]);
                }
                else
                {
                    //If command is out of bounds or does not exist, ignore
                    printf("Command not in history.\n");
                    break;
                }
            }

            //Store history to print and increment counter to track size of history
            strcpy(history[count%15], cmd_str);
            count++;
                
            //Get the name of the executable
            char* token = strtok(cmd_str, WHITESPACE);
            
            //Use to store the arguments of the executable
            char* args[MAX_ARGS_SIZE];
            int token_count = 0;
                    
            //Parse the string into tokens until the max count is reached
            //Use the tokens to pass as parameters for execvp()
            while(token != NULL && token_count < MAX_ARGS_SIZE)
            {
                args[token_count] = (char*)malloc(MAX_INPUT_SIZE);
                strcpy(args[token_count], token);
                if(strlen(args[token_count]) == 0)
                {
                    args[token_count] = NULL;
                }
                token = strtok(NULL, WHITESPACE);
                token_count++;
            }
            //Set all remaining arguments to NULL since there must be a NULL when using exec
            for(i = token_count; i < MAX_ARGS_SIZE; i++)
            {
                args[token_count++] = NULL;     
            }
            
            //Run commands entered that do not require fork
            if(strcmp(args[0], "quit") == 0 || strcmp(args[0], "exit") == 0)
            {
                //Free all mallocs before exiting
                free(cmd_str);
                for(i = 0; i < 15; i++)
                {
                    free(history[i]);
                }
                for(i = 0; i < token_count; i++)
                {
                    free(args[i]);
                }
                return 0;
            }
            else if(strcmp(args[0], "showpids") == 0)
            {
                //Shows last 15 PID ID's. Note: wraps around after 15
                printf("PID count = %d\n", pid_count);
                for(i = 0; i < pid_count && i < 15; i++)
                {
                    printf("PID %d: %d\n", i, pids[i]);
                }
                break;
            }
            else if(strcmp(args[0], "history") == 0)
            {
                //Shows last 15 commands. Note: wraps around after 15
                for(i = 0; i < count && i < 15; i++)
                {
                    printf("%d: %s", i+1, history[i]);
                }
                break;
            }
            else if(strcmp(args[0], "cd") == 0)
            {
                //Changes the directory. Will only change directory if arg count greater than 1
                //Typing cd will not move anywhere
                chdir(args[1]);
                break;
            }
        
            //Attempt to run the child process and execute
            pid_t pid = fork();
            if(pid == 0)
            {       
                int ret = execvp(args[0], args);

                //If the exec failed, print out which command failed
                if(ret == -1)
                {
                    printf("%s: Command not found.\n", args[0]);
                }
                return 0;
                exit(EXIT_SUCCESS);
            }
            else if(pid == -1)
            {
                //If the fork failed, exit the program to prevent further issues
                perror("Fork failed: ");
                exit(EXIT_FAILURE);
            }
            else
            {
                int status;
                //Wait for child process to finish before allowing another command
                waitpid(pid, &status, 0);
                
                //Store the pids. Wraps after 15 pids inserted
                pids[pid_count++%15] = pid;
                
                //Reset arguments and counter for the next possible execution
                //Prevent the program from reusing previous arguements
                for(i = 0; i < token_count; i++)
                {
                    free(args[i]);
                }
                token_count = 0;
                break;
            }
        }
    } 
}
