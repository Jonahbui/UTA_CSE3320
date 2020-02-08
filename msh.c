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
	        break;

            //Store history to print and increment the counter to understand current length of history
            strcpy(history[count++], cmd_str);
	    
	    //Get the name of the executable
	    char* token = strtok(cmd_str, WHITESPACE);
	    char* exec_name = token;
	    token = strtok(NULL, WHITESPACE);

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

	    //Check if certain commands
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
	    else if(strcmp(exec_name, "history") == 0)
	    {
	    	for(int i = 0; i < 15; i++)
		{
		    printf("%d: %s", i, history[i]);
		}
	    }
	    else if(strcmp(exec_name, "cd") == 0)
	    {
	        //chdir(args);
	    }
	    
	    //Run the child process
	    pid_t pid = fork();
	    if(pid == 0)
	    {	    
		int ret = execvp(exec_name, args);

		if(ret == -1)
		{
		    printf("Failed to execute!\n");
		}
		return 0;
		exit(EXIT_SUCCESS);
	    }
	    else
	    {
	    	int status;
		wait(&status);
		break;
	    }
	}
    } 
}
