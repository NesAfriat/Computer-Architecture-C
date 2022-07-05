#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#define NAME_LEN 128
#define BUF_SZ 10000
void print_units_Hex(int unit, int value);
void print_units_Dec(int unit, int value);
typedef struct
{
    char debug_mode;
    char file_name[NAME_LEN];
    int unit_size;
    unsigned char mem_buf[BUF_SZ];
    size_t mem_count;
    /*
   .
   .
   Any additional fields you deem necessary
  */
} state;
struct fun_desc
{
    char *name;
    void (*fun)(state *s);
};
void map(state *s, void (*f)(state *s))
{
    f(s);
}
void ToggleDebugMode(state *s)
{
    if (s->debug_mode == 0)
    {
        s->debug_mode = 1;
        printf("Debug flag now on\n");
    }
    else
    {
        s->debug_mode = 0;
        printf("Debug flag now off\n");
    }
}
void SetFileName(state *s)
{
    char fileName[100];
    printf("Enter file name\n");
    fgets(fileName, 100, stdin);
    sscanf(fileName, "%s", (s->file_name));
    if (s->debug_mode == 1)
        fprintf(stderr, "Debug: file name set to '%s' \n", s->file_name);
}
void SetUnitSize(state *s)
{
    char us[4];
    int size;
    printf("Enter size\n");
    fgets(us, 4, stdin);
    sscanf(us, "%d", &size);
    if (size == 1 || size == 2 || size == 4)
    {
        s->unit_size = size;
        if (s->debug_mode == 1)
        {
            fprintf(stderr, "Debug: set size to %d\n", size);
        }
    }
    else
    {
        printf("Error - invalid size\n");
    }
}
void LoadInMem(state *s)
{
    char locationBuff[20];
    char lengthBuff[10000];
    int size;
    unsigned int offset;
    if (strcmp(s->file_name, "") == 0)
    {
        printf("Error trying to open an empty filename");
        return;
    }
    int fd = open(s->file_name, O_RDONLY);
    if (fd == -1)
    {
        printf("failed to open a file");
        return;
    }
    printf("please enter the location in hex (0x..)\n");
    fgets(locationBuff, 20, stdin);
    sscanf(locationBuff, "%x", &offset);
    printf("please enter the length in dec\n");
    fgets(lengthBuff, 10000, stdin);
    sscanf(lengthBuff, "%d", &size);
    if (s->debug_mode == 1)
    {
        fprintf(stderr, "File name: %s\n", s->file_name);
        fprintf(stderr, "Location: %x\n", offset);
        fprintf(stderr, "Length: %d\n", size);
    }
    lseek(fd, offset, SEEK_SET);
    int num = read(fd, &s->mem_buf, size * s->unit_size);
    printf("Loaded %d units into memory\n", size * s->unit_size);
    s->mem_count = num;
    close(fd);
}
void MemDisp(state *s)
{
    char uBuf[100];
    char addrBuf[20];
    char *readBuff;
    char *readBuff2;
    int u;
    int addr;
    printf("please enter the address in hex (0x..)\n");
    fgets(addrBuf, 20, stdin);
    sscanf(addrBuf, "%x", &addr);
    printf("please enter the length in dec\n");
    fgets(uBuf, 100, stdin);
    sscanf(uBuf, "%d", &u);
    if (s->debug_mode == 1)
    {
        fprintf(stderr, "File name: %s\n", s->file_name);
        fprintf(stderr, "addres: %x\n", addr);
        fprintf(stderr, "units: %d\n", u);
    }
    if (addr == 0)
    {
        readBuff = (char *)s->mem_buf;
    }
    else
    {
        readBuff = (char *)addr;
    }
    readBuff2 = readBuff;
    char *end = readBuff + u * (s->unit_size);
    printf("Hexadecimal\n===========\n");
    while (readBuff < end)
    {
        int var = *((int *)(readBuff));
        print_units_Hex(s->unit_size, var);
        readBuff += s->unit_size;
    }
    printf("\nDecimal\n=======\n");
    while (readBuff2 < end)
    {
        int var = *((int *)(readBuff2));
        print_units_Dec(s->unit_size, var);
        readBuff2 += s->unit_size;
    }
}

void print_units_Hex(int unit, int value)
{
    static char *formatsHex[] = {"%hhX\n", "%hX\n", "No such unit", "%X\n"};
    printf(formatsHex[unit - 1], value);
}
void print_units_Dec(int unit, int value)
{
    static char *formatsDec[] = {"%hhd\n", "%hd\n", "No such unit", "%d\n"};
    printf(formatsDec[unit - 1], value);
}
void SaveInFile(state *s)
{
    char sourceAddrBuf[20];
    char targetAddrBuf[20];
    char lengthBuf[100];
    char *readBuff;
    int len;
    int sAddr;
    int tAddr;
    printf("please enter the source address in hex (0x..)\n");
    fgets(sourceAddrBuf, 20, stdin);
    sscanf(sourceAddrBuf, "%x", &sAddr);
    printf("please enter the target address in hex (0x..)\n");
    fgets(targetAddrBuf, 20, stdin);
    sscanf(targetAddrBuf, "%x", &tAddr);
    printf("please enter the length in dec\n");
    fgets(lengthBuf, 100, stdin);
    sscanf(lengthBuf, "%d", &len);
    if (s->debug_mode == 1)
    {
        fprintf(stderr, "File name: %s\n", s->file_name);
        fprintf(stderr, "source address: %x\n", sAddr);
        fprintf(stderr, "target address: %x\n", tAddr);
        fprintf(stderr, "Length: %d\n", len);
    }
    if (sAddr == 0)
    {
        readBuff = (char *)s->mem_buf;
    }
    else
    {
        readBuff = (char *)sAddr;
    }
    int fd = open(s->file_name, O_WRONLY);
    if (fd == -1)
    {
        printf("Error trying to open the file\n");
        return;
    }
    lseek(fd, 0, SEEK_END);
    if (tAddr > lseek(fd, 0, SEEK_CUR))
    {
        printf("Error target location is greater the size of the file\n");
        return;
    }
    lseek(fd, tAddr, SEEK_SET);
    write(fd, readBuff, s->unit_size * len);
    close(fd);
}
void MemModify(state *s)
{
    char locationBuf[20];
    char valBuf[20];
    int val;
    int location;
    char *memoStart = (char *)s->mem_buf;
    printf("please enter the location in hex (0x..)\n");
    fgets(locationBuf, 20, stdin);
    sscanf(locationBuf, "%x", &location);
    char *formatsHex[] = {"%hhX\n", "%hX\n", "No such unit", "%X\n"};
    printf("please enter the new val in hex\n");
    fgets(valBuf, 20, stdin);
    sscanf(valBuf, formatsHex[s->unit_size - 1], &val);
    if (s->debug_mode == 1)
    {
        fprintf(stderr, "File name: %s\n", s->file_name);
        fprintf(stderr, "offset in mem_buf: %x\n", location);
        fprintf(stderr, "value: %x\n", val);
    }
    if (location >= BUF_SZ)
    {
        printf("Error trying to modify the memory array\n");
        return;
    }

    memoStart = memoStart + location;
    memcpy(memoStart, &val, s->unit_size);
}
void quit(state *s)
{
    if (s->debug_mode == 1)
        fprintf(stderr, "quitting\n");
    free(s);
    exit(0);
}
int main(int argc, char **argv)
{
    int c;
    char buff[4];
    state *st = calloc(1, sizeof(state));
    struct fun_desc menu[] = {{"Toggle Debug Mode", ToggleDebugMode},
                              {"Set File Name", SetFileName},
                              {"Set Unit Size", SetUnitSize},
                              {"Load Into Memory", LoadInMem},
                              {"Memory Display", MemDisp},
                              {"Save Into File", SaveInFile},
                              {"Memory Modify", MemModify},
                              {"Quit", quit},
                              {NULL, NULL}};

    while (1)
    {
        if (st->debug_mode == 1)
        {
            fprintf(stderr, "Unit size: %d\n", st->unit_size);
            fprintf(stderr, "File name: %s\n", st->file_name);
            fprintf(stderr, "Memory count: %d\n", st->mem_count);
        }
        printf("\nChoose action:\n");
        for (int i = 0; menu[i].name != NULL; i++)
            printf("%d-%s\n", i, menu[i].name);
        fgets(buff, 4, stdin);
        sscanf(buff, "%d", &c);
        if ((c < 8) & (c > -1))
        {
            map(st, menu[c].fun);
        }
        else
        {
            printf("Option: %d\nNot Within bounds\n", c);
            free(st);
            exit(0);
        }
    }
}