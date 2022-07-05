#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/limits.h>
#include "LineParser.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>

void execute(cmdLine *pCmdLine){
    if(strcmp(pCmdLine->arguments[0],"cd")==0){
        if(chdir(pCmdLine->arguments[1])==-1){
            fprintf(stderr,"error using cd to dir: %s\n", pCmdLine->arguments[1]);
        }
    }
    else{
        int cpid=fork();
        if(cpid==0){ /*child code*/
            execvp(pCmdLine->arguments[0],pCmdLine->arguments);
            _exit(0);
            
        }
        else{ /*parent code*/
            if (pCmdLine->blocking == 1)
            {
                waitpid(cpid, NULL, WUNTRACED);
            }
        }
    }
    if(pCmdLine!=NULL)freeCmdLines(pCmdLine);
}
int main(int argc, char **argv) {
    char buf_path[PATH_MAX], buf[2048];
    cmdLine* cmdline;
    while(1){
        printf("the current working dir is: %s\n",getcwd(buf_path, PATH_MAX));
        fgets(buf,2048,stdin);
        if(strncmp(buf,"quit",4)==0){
            exit(0);
        }
        cmdline = parseCmdLines(buf);
        execute(cmdline);

    }
    return 0;
}