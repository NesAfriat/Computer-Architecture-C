#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv)
{
    int i, debug_flag = 0;
    for (i = 1; i < argc; i++)
    {
        strcmp("-d", argv[i]) == 0 ? debug_flag = 1 : 0;
    }

    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        perror("error in pipe");
        _exit(0);
    }
    if (debug_flag == 1)
        fprintf(stderr, "%d >forking...\n", getpid());
    int pid1 = fork();
    if (pid1 != 0 && debug_flag == 1)
        fprintf(stderr, "%d >created process with id: %d\n", getpid(), pid1);
    if (pid1 == 0)
    { /*child 1 code */
        char *arr[] = {"ls", "-l", 0};
        close(STDOUT_FILENO);
        int dupfd = dup2(pipefd[1],STDOUT_FILENO);
        if (dupfd == -1)
        {
            perror("error dup1");
            _exit(0);
        }
        if (debug_flag == 1)
            fprintf(stderr, "child1 >redirecting stdout to the write end of the pipe...\n");
        close(pipefd[1]);
        if (debug_flag == 1)
            fprintf(stderr, "child1 >going to execute cmd: %s\n", arr[0]);
        int err = execvp(arr[0], arr);
        if (err == -1)
        {
            perror("error in child 1");
            _exit(1);
        }
        _exit(1);
    }
    else
    { /* parent code */
        if (debug_flag == 1)
            fprintf(stderr, "%d >closing the write end of the pipe...\n", getpid());
        close(pipefd[1]);
        int pid2 = fork();
        if (pid2 == 0)
        { /* child 2 code */
            char *arr[] = {"tail", "-n", "2", 0};
            close(STDIN_FILENO);
            int dupfd = dup2(pipefd[0],STDIN_FILENO);
            if (dupfd == -1)
            {
                perror("error dup2");
                _exit(0);
            }
            if (debug_flag == 1)
                fprintf(stderr, "child2 >redirecting stdin to the write end of the pipe...\n");
            close(pipefd[0]);
            if (debug_flag == 1)
                fprintf(stderr, "child2 >going to execute cmd: %s\n", arr[0]);
            int err = execvp(arr[0], arr);
            if (err == -1)
            {
                perror("error in child 2");
                _exit(1);
            }
        }
        else
        { /* parend code */
            if (debug_flag == 1)
                fprintf(stderr, "%d >waiting for child1 process to terminate...\n", getpid());
            waitpid(pid1, NULL, WUNTRACED);
            if (debug_flag == 1)
                fprintf(stderr, "%d >waiting for child2 process to terminate...\n", getpid());
            waitpid(pid2, NULL, WUNTRACED);
            if (debug_flag == 1)
                fprintf(stderr, "%d >closing the read end of the pipe...\n", getpid());
            close(pipefd[0]);
            if (debug_flag == 1)
                fprintf(stderr, "%d >exiting...\n", getpid());
        }
    }
}