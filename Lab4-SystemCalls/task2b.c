#include "util.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "/usr/include/dirent.h"

#define STDIN 0
#define STDOUT 1
#define STDERR 2

#define SYS_READ 3
#define SYS_ClOSE 6
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_SEEK 19
#define SEEK_SET 0
#define sys_getdents 141
#define SYS_EXIT 1

#define DT_UNKNOWN 0
#define DT_FIFO 1
#define DT_CHR 2
#define DT_DIR 4
#define DT_BLK 6
#define DT_REG 8
#define DT_LNK 10
#define DT_SOCK 12


extern int system_call();

typedef struct ent
{
  int inode;
  int offset;
  short len;
  char buf[];
} ent;

const char *getType(char type)
{
  char *returnType;
  (type == DT_REG) ? returnType = "regular" : (type == DT_DIR) ? returnType = "directory" : (type == DT_FIFO) ? returnType = "FIFO" : (type == DT_SOCK) ? returnType = "socket" : (type == DT_LNK) ? returnType = "symlink" : (type == DT_BLK) ? returnType = "block dev" : (type == DT_CHR) ? returnType = "char dev" : "???";
  return returnType;
}

int main(int argc, char *argv[], char *envp[])
{
  int i, fd, withPref = 0;
  int output = STDOUT;
  char prefix;
  char type;
  char buff[8192];
  ent *entp;
  int count = 0;
  fd = system_call(SYS_OPEN, ".", O_RDONLY, 0644);
  if (fd < 0)
  {
    /* system_call(SYS_ERR,STDOUT,errno,10); */
    system_call(SYS_WRITE, STDOUT, "Error opening file occurd!\n", 30);
    system_call(SYS_EXIT, 0x55);
  }
  for (i = 1; i < argc; i++)
  {
    strcmp("-D", argv[i]) == 0 ? output = STDERR : output;
    if (strncmp(argv[i], "-p", 2) == 0)
    {
      prefix = argv[i][2];
      withPref = 1;
    }
  }
  count = system_call(sys_getdents, fd, buff, 8192);
  if (count < 0)
  {
    /* system_call(SYS_ERR,STDOUT,errno,10); */
    system_call(SYS_WRITE, STDOUT, "Error occurd reading from file!\n", 40);
    system_call(SYS_EXIT, 0x55);
  }
  int tmp = 0;
  while (tmp < count)
  {
    entp = (ent *)(buff + tmp);
    if (withPref == 0)
    {
      system_call(SYS_WRITE, output, (entp)->buf, strlen(entp->buf));
      system_call(SYS_WRITE, output, "\n", 1);
      type = *(buff + tmp + (entp)->len - 1);
      const char *typeOF = getType(type);
      system_call(SYS_WRITE, output, "\t", 1);
      system_call(SYS_WRITE, output, typeOF, strlen(typeOF));
    
    }
    else if (withPref == 1 && ((entp)->buf[0] == prefix))
    {
      system_call(SYS_WRITE, output, (entp)->buf, strlen(entp->buf));
      type = *(buff + tmp + (entp)->len - 1);
      const char *typeOF = getType(type);
      system_call(SYS_WRITE, output, "\t", 1);
      system_call(SYS_WRITE, output, typeOF, strlen(typeOF));
      system_call(SYS_WRITE, output, "\n", 1);
    }
    tmp = tmp + entp->len;
  }
  fd = system_call(SYS_ClOSE, output);
  if (fd < 0)
  {
    system_call(SYS_WRITE, STDOUT, "Error occurd Closing!\n", 22);
    system_call(SYS_EXIT, 0x55);
  }
  return 0;
}