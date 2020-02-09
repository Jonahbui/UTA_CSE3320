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
    char* cmd_str = (char*)malloc(MAX_INPUT_SIZE);
    
    //Store the ID of child processes and keep track of how many have spawned
    int pids[15];
    int pid_count = 0;

    //Use to store history
    char* history[15];
    int i;
    static int count = 0;
    for(i = 0; i < 15; i++)
    {
        history[i] = (char*)malloc(MAX_INPUT_SIZE);
        memset(history[i], 0, MAX_INPUT_SIZE);
    }

    //Loop the program until the user quits/exits
    while(1)
    {
        printf("msh> ");
        while(fgets(cmd_str, MAX_INPUT_SIZE, stdin) != NULL)
        {
            //Ignore input if it was just ENTER
            if(strcmp(cmd_str, "\n") == 0)
            {
                break;
            }
            else if(strpbrk(cmd_str, "!1234567890") != NULL)
            {
                char* token = strtok(cmd_str, "!");
                int history_num = atoi(token)-1;
                if(history_num <= count)
                {
                    strcpy(cmd_str, history[history_num]);
                }
                else
                {
                    printf("Command not in history.\n");
                    break;
                }
            }

            //Store history to print and increment the counter to understand current length of history
            strcpy(history[count%15], cmd_str);
            count++;
                
            //Get the name of the executable
            char* token = strtok(cmd_str, WHITESPACE);
            char* exec_name = token;
            
            //Use to store the arguments of the executable
            char* args[MAX_ARGS_SIZE];
            int token_count = 0;
                    
            //Parse the string into tokens until the max count is reached
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
            //Set all remaining arguments to NULL
            for(i = token_count; i < MAX_ARGS_SIZE; i++)
            {
            args[token_count++] = NULL;     
            }
            
            //Check if certain commands are entered that do not require fork
            if(strcmp(exec_name, "quit") == 0 || strcmp(exec_name, "exit") == 0)
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
            else if(strcmp(exec_name, "showpids") == 0)
            {
                printf("PID count = %d\n", pid_count);
                for(i = 0; i < pid_count && i < 15; i++)
                {
                    printf("PID %d: %d\n", i, pids[i]);
                }
                break;
            }
            else if(strcmp(exec_name, "history") == 0)
            {
                for(i = 0; i < count && i < 15; i++)
                {
                    printf("%d: %s", i+1, history[i]);
                }
                break;
            }
            else if(strcmp(exec_name, "cd") == 0)
            {
                chdir(args[1]);
                break;
            }
        
            //Run the child process
            pid_t pid = fork();
            if(pid == 0)
            {       
                int ret = execvp(exec_name, args);
                if(ret == -1)
                {
                    printf("%s: Command not found.\n", exec_name);
                }
                return 0;
                exit(EXIT_SUCCESS);
            }
            else if(pid == -1)
            {
                perror("Fork failed: ");
                exit(EXIT_FAILURE);
            }
            else
            {
                int status;
                waitpid(pid, &status, 0);
                
                pids[pid_count++%15] = pid;
                
                //Reset arguments for the next possible execution
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
