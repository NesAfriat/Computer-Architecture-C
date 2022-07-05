#include "util.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
/*#include "errno.h"*/

#define SYS_EXIT 1
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define SYS_READ 3
#define SYS_WRITE 4
#define SYS_LSEEK 19

#define STDIN 0
#define STDOUT 1
#define STDERR 2

/* extern int system_call(int arg1, int arg2, char* arg3, int arg4);*/
extern int system_call();
int main(int argc, char *argv[], char *envp[])
{
    int i;
    char *file;
    int fdIn = STDIN;
    int fdOut = STDOUT;
    for (i = 1; i < argc; i++)
    {
        if (strncmp(argv[i], "-i", 2) == 0)
        {
            file = &argv[i][2];
            fdIn = system_call(SYS_OPEN, file, O_RDONLY, 0777);
            if (fdIn < 0)
            {
                /* system_call(SYS_ERR,STDOUT,errno,10); */
                system_call(SYS_WRITE, STDOUT, "Error occurd!\n", 14);
                system_call(SYS_EXIT, 0x55);
            }
        }
        if (strncmp(argv[i], "-o", 2) == 0)
        {
            file = &argv[i][2];
            fdOut = system_call(SYS_OPEN, file, O_RDWR | O_CREAT, 0777);
            if (fdOut < 0)
            {
                /* system_call(SYS_ERR,STDOUT,errno,10); */
                system_call(SYS_WRITE, STDOUT, "Error occurd!\n", 14);
                system_call(SYS_EXIT, 0x55);
            }
        }
    }

    char buf;
    while (system_call(SYS_READ, fdIn, &buf, 1) != 0)
    {
        buf >= 'A' &&buf <= 'Z' ? buf = buf + ('a' - 'A') : buf;
        system_call(SYS_WRITE, fdOut, &buf, 1);
    }
    fdIn = system_call(SYS_CLOSE, fdIn);
    if (fdIn < 0)
    {
        system_call(SYS_WRITE, STDOUT, "Error occurd Closing!\n", 22);
        system_call(SYS_EXIT, 0x55);
    }
    fdOut = system_call(SYS_CLOSE, fdOut);
    if (fdOut < 0)
    {
        system_call(SYS_WRITE, STDOUT, "Error occurd Closing!\n", 22);
        system_call(SYS_EXIT, 0x55);
    }
    return 0;
}
