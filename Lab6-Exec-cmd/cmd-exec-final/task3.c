#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

#define list_len 10
typedef struct cmdList
{
    cmdLine *arr[list_len];
    int arrMult[list_len];
    int count;
} cmdList;

cmdList *list;
void checkRedirection(cmdLine *cmd);
void addToList(cmdLine *cmd);
void freeList();

void freeList()
{
    for (int i = 0; i < list->count; i++)
    {
        if(list->arrMult[i]==0) freeCmdLines(list->arr[i]);
    }
    free(list);
}
void addToList(cmdLine *cmd)
{
    if (list->count < list_len)
    {
        // list->arr[list->count] = malloc(sizeof(cmdLine));
        list->arr[list->count] = cmd;
        list->arrMult[list->count] = 0;
        list->count++;
    }
    else
    {
        printf("could not store more cmd");
    }
}
void execute(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        if (chdir(pCmdLine->arguments[1]) == -1)
        {
            fprintf(stderr, "error using cd to dir: %s\n", pCmdLine->arguments[1]);
        }
        addToList(pCmdLine);
    }
    else if (strncmp(pCmdLine->arguments[0], "!", 1) == 0)
    {
        int index;
        char *action = pCmdLine->arguments[0] + 1;
        sscanf(action, "%d", &index);
        if (index <= list->count && index < list_len)
        {
            list->arrMult[index] = 1;
            execute(list->arr[index]);
        
        }
        else
        {
            printf("error with ! index\n");
        }
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "history") == 0)
    {
        for (int i = 0; i < list->count; i++)
        {
            printf("#%d\t", i);
            for (int j = 0; j < list->arr[i]->argCount; j++)
            {
                printf("%s ", list->arr[i]->arguments[j]);
            }
            if (list->arr[i]->next != NULL)
            {
                printf("| ");
                for (int p = 0; p < list->arr[i]->next->argCount; p++)
                {
                    printf("%s ", list->arr[i]->next->arguments[p]);
                }
            }
            printf("\n");
        }
        addToList(pCmdLine);
    }
    else
    {
        if (pCmdLine->next != NULL)
        { //we have pipline
            int pipefd[2];
            pipe(pipefd);
            int cpid1 = fork();
            if (cpid1 == 0)
            { /*child 1 code */
                checkRedirection(pCmdLine);
                close(STDOUT_FILENO);
                int dupfd = dup(pipefd[1]);
                if (dupfd == -1)
                {
                    perror("error dup1");
                    _exit(0);
                }
                close(pipefd[1]);
                int err = execvp(pCmdLine->arguments[0], pCmdLine->arguments);
                if (err == -1)
                {
                    perror("error in child 1");
                    _exit(1);
                }
            }
            else
            { /* parent code */
                waitpid(cpid1, NULL, WUNTRACED);
                close(pipefd[1]);
                int cpid2 = fork();
                if (cpid2 == 0)
                { /* child 2 code */
                    cmdLine *nextCmd = pCmdLine->next;
                    checkRedirection(nextCmd);
                    close(STDIN_FILENO);
                    int dupfd = dup(pipefd[0]);
                    if (dupfd == -1)
                    {
                        perror("error dup2");
                        _exit(0);
                    }
                    close(pipefd[0]);
                    int err = execvp(nextCmd->arguments[0], nextCmd->arguments);
                    if (err == -1)
                    {
                        perror("error in child 2");
                        _exit(1);
                    }
                }
                else
                { /* parend code */
                    waitpid(cpid2, NULL, WUNTRACED);
                    close(pipefd[0]);
                    addToList(pCmdLine);
                }
            }
        }
        else
        { /* if there is no pipe*/
            int cpid = fork();
            if (cpid == 0)
            { /*child code*/
                checkRedirection(pCmdLine);
                execvp(pCmdLine->arguments[0], pCmdLine->arguments);
                _exit(0);
            }
            else
            { /*parent code*/
                if (pCmdLine->blocking == 1)
                {
                    addToList(pCmdLine);
                    waitpid(cpid, NULL, WUNTRACED);
                }
            }
        }
    }
}

void checkRedirection(cmdLine *cmd)
{
    if (cmd->inputRedirect != NULL)
    {
        freopen(cmd->inputRedirect, "r+", stdin);
        // close(STDIN_FILENO);
        // open(cmd->inputRedirect, O_RDONLY, 0644);
    }
    if (cmd->outputRedirect != NULL)
    {
        freopen(cmd->outputRedirect, "w+", stdout);
        // close(STDOUT_FILENO);
        // open(cmd->outputRedirect, O_WRONLY, 0644);
    }
}
int main(int argc, char **argv)
{
    char buf_path[PATH_MAX], buf[2048];
    cmdLine *cmdline;
    list = malloc(sizeof(cmdList));
    list->count = 0;
    while (1)
    {
        printf("the current working dir is: %s\n", getcwd(buf_path, PATH_MAX));
        fgets(buf, 2048, stdin);
        if (strncmp(buf, "quit", 4) == 0)
        {
            freeList();
            exit(0);
        }
        cmdline = parseCmdLines(buf);
        execute(cmdline);
    }
    return 0;
}