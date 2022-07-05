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

void checkRedirection(cmdLine *cmd);
void execute(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        if (chdir(pCmdLine->arguments[1]) == -1)
        {
            fprintf(stderr, "error using cd to dir: %s\n", pCmdLine->arguments[1]);
        }
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
                    waitpid(cpid, NULL, WUNTRACED);
                }
            }
        }
        if (pCmdLine != NULL)
            freeCmdLines(pCmdLine);
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
    while (1)
    {
        printf("the current working dir is: %s\n", getcwd(buf_path, PATH_MAX));
        fgets(buf, 2048, stdin);
        if (strncmp(buf, "quit", 4) == 0)
        {
            exit(0);
        }
        cmdline = parseCmdLines(buf);
        execute(cmdline);
    }
    return 0;
}