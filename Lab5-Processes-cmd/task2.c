#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

process *p_lst = NULL; /*global list*/

void printProcessList(process **process_list);
void addProcess(process **process_list, cmdLine *cmd, pid_t pid);
void freeProcessList(process *process_list);
void updateProcessList(process **process_list);
void updateProcessStatus(process *process_list, int pid, int status);
void deleteProcess(process **process_list, process *procToDel);
void execute(cmdLine *pCmdLine);
void kill_all(process *process_list);

void execute(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        if (chdir(pCmdLine->arguments[1]) == -1)
        {
            fprintf(stderr, "error using cd to dir: %s\n", pCmdLine->arguments[1]);
        }
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "procs") == 0)
    {
        printProcessList(&p_lst);
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "free") == 0)
    {
        freeProcessList(p_lst);
        p_lst = NULL;
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "kill") == 0)
    {
        kill(atoi(pCmdLine->arguments[1]), SIGINT);
        freeCmdLines(pCmdLine);
    }
    else if (strcmp(pCmdLine->arguments[0], "suspend") == 0)
    {
        int cpid = fork();
        if (cpid == 0)
        {
                                               
            kill(atoi(pCmdLine->arguments[1]), SIGTSTP); 
            sleep(atoi(pCmdLine->arguments[2]));
            kill(atoi(pCmdLine->arguments[1]), SIGCONT); 
            freeCmdLines(pCmdLine);
            freeProcessList(p_lst); /*ive added this*/
            p_lst = NULL;
            _exit(0);
            
        }
        else
        {
            // waitpid(cpid, 0, WUNTRACED | WCONTINUED);
            freeCmdLines(pCmdLine);
            // _exit(0);
        }
    }
    else
    {
        int cpid = fork();
        if (cpid == 0)
        { /*child code*/
            execvp(pCmdLine->arguments[0], pCmdLine->arguments);
            freeProcessList(p_lst);
            p_lst=NULL;
            freeCmdLines(pCmdLine);
            _exit(0);
        }
        else
        {
            /* parent code */
            addProcess(&p_lst, pCmdLine, cpid); /* father add his new child to the proc_list */
            if (pCmdLine->blocking == 1)
            {
                waitpid(cpid, NULL, WUNTRACED);
            }
        }
    }
}

void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    process *pros = malloc(sizeof(process));
    pros->cmd = cmd;
    pros->pid = pid;
    pros->status = RUNNING;
    pros->next = *process_list;
    *process_list = pros;
}

void printProcessList(process **process_list)
{
    if (process_list != NULL)
    {
        int flag = 0;
        updateProcessList(process_list);
        printf("PID\tCommand\tSTATUS\n");
        process *tmp = *process_list;
        process *tail = NULL;
        while (tmp != NULL)
        {
            char *str;
            switch (tmp->status)
            {
            case -1:
            {
                str = "TERMINATED";
                flag = 1;
                break;
            }
            case 1:
            {
                str = "RUNNING";
                break;
            }
            case 0:
            {
                str = "SUSPENDED";
                break;
            }
            }
            printf("%d\t%s\t%s\n", tmp->pid, tmp->cmd->arguments[0], str);
            if (flag == 1)
            {
                if (tail == NULL)
                {
                    tail = tmp;
                    tmp = tmp->next;
                    freeCmdLines(tail->cmd);
                    free(tail);
                    tail = NULL;
                    *process_list = tmp;
                }
                else
                {
                    tail->next = tmp->next;
                    freeCmdLines(tmp->cmd);
                    free(tmp);
                    tmp = tail;
                }
                flag = 0;
            }
            else
            {
                tail = tmp;
                tmp = tmp->next;
                flag = 0;
            }
        }
    }
}

void freeProcessList(process *process_list)
{
    if (process_list == NULL)
    {
        return;
    }
    else
    {
        freeProcessList(process_list->next);
        freeCmdLines(process_list->cmd);
        free(process_list);
    }
}

void updateProcessList(process **process_list)
{
    if (*process_list == NULL)
        return;
    int status=0;
    int pid=0;
    process *curProc = *process_list;
    pid = waitpid(curProc->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
    pid == -1 ? updateProcessStatus(curProc,curProc->pid,TERMINATED):0;
    if (pid)
    {
    if (WIFSIGNALED(status)| WIFEXITED(status))
    {
        updateProcessStatus(curProc,curProc->pid,TERMINATED);
    }
    else if (WIFSTOPPED(status))
    {
        updateProcessStatus(curProc,curProc->pid,SUSPENDED);
    }
    else if (WIFCONTINUED(status))
    {
        updateProcessStatus(curProc,curProc->pid,RUNNING);
    }
    updateProcessList(&curProc->next);
    }
}

void updateProcessStatus(process *process_list, int pid, int status)
{
    process_list->status = status;
}

void kill_all(process *process_list)
{
    if (process_list != NULL)
    {
        process *tmp = process_list;
        while (tmp != NULL)
        {
            if ((tmp->status== RUNNING) || (tmp->status==SUSPENDED))
            {
                kill(tmp->pid, SIGINT);
            }
            tmp = tmp->next;
        }
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
            // kill_all(p_lst);
            freeProcessList(p_lst);
            p_lst = NULL;
            exit(0);
        }
        cmdline = parseCmdLines(buf);
        execute(cmdline);
    }
    return 0;
}