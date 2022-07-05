#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <elf.h>
#include <sys/mman.h>

#define NAME_LEN 128

int Currentfd = -1; /* global variable for the current open file */
void *map_start;    /*global variable to indicate memory location */
char debug_mode = 0;
struct stat fd_stat;
Elf32_Ehdr *elfHeader = NULL;
Elf32_Shdr *elfSectionsHeader;

struct fun_desc
{
    char *name;
    void (*fun)();
};
void map(void (*f)())
{
    f();
}
void ToggleDebugMode()
{
    if (debug_mode == 0)
    {
        debug_mode = 1;
        printf("Debug flag now on\n");
    }
    else
    {
        debug_mode = 0;
        printf("Debug flag now off\n");
    }
}
void ExamineELF()
{
    char file_name[100];
    if (Currentfd != -1)
        close(Currentfd);
    char fileNameBuf[100];
    printf("Enter file name\n");
    fgets(fileNameBuf, 100, stdin);
    sscanf(fileNameBuf, "%s", (file_name));
    if (debug_mode == 1)
        fprintf(stderr, "Debug: file name set to '%s' \n", file_name);
    Currentfd = open(file_name, O_RDWR);
    if (Currentfd == -1)
    {
        perror("open failed");
        return;
    }
    if (fstat(Currentfd, &fd_stat) != 0)
    {
        perror("stat failed");
        close(Currentfd);
        return;
    }
    if ((map_start = mmap(0, fd_stat.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, Currentfd, 0)) == MAP_FAILED)
    {
        printf("mmap failed");
        close(Currentfd);
        Currentfd = -1;
        return;
    }
    elfHeader = (Elf32_Ehdr *)map_start;
    elfSectionsHeader = (Elf32_Shdr *)(map_start + elfHeader->e_shoff);
    printf("Magic numbers: %s\n", elfHeader->e_ident + 1);
    int e_type = elfHeader->e_ident[5];
    switch (e_type)
    {
    case 0:
        printf("Data encoding scheme: Invalid data encoding\n");
        break;
    case 1:
        printf("Data encoding scheme: 2's complement, little endian\n");
        break;
    case 2:
        printf("Data encoding scheme: 2's complement, big endian\n");
        break;
    default:
        break;
    }
    printf("Entry point: 0x%x\n", elfHeader->e_entry);
    if (strncmp((const char *)elfHeader->e_ident + 1, "ELF", 3) != 0)
    {
        printf("Not an elf file");
        close(Currentfd);
        Currentfd = -1;
        return;
    }
    printf("File offset of section header table: %d\n", elfHeader->e_shoff);
    printf("Number of section header entries: %d\n", elfHeader->e_shnum);
    printf("The size of each section header entry: %d\n", elfHeader->e_shentsize);
    printf("File offset-in which the program header table resides: %d\n", elfHeader->e_phoff);
    printf("Number of program header entries: %d\n", elfHeader->e_phnum);
    printf("The size of each program header entry: %d\n", elfHeader->e_phentsize);
}
void PrintSection()
{
    Elf32_Shdr *tmp = &elfSectionsHeader[elfHeader->e_shstrndx];
    const char *const p = map_start + tmp->sh_offset;
    if (debug_mode == 1)
    {
        fprintf(stderr, "offset of Section header string table index: %d\n", elfHeader->e_shstrndx);
    }
    printf("index\tname\t\t\taddres\t\toffset\tsize\ttype\n");
    fflush(stdout);
    for (int i = 0; i < elfHeader->e_shnum; i++)
    {
        if (debug_mode == 1)
        {
            fprintf(stderr, "offset of name: %d\n", elfSectionsHeader[i].sh_name);
        }
        printf("[%d]\t", i);
        printf("%-20s\t", p + elfSectionsHeader[i].sh_name);
        printf("0x%-10x\t", elfSectionsHeader[i].sh_addr);
        printf("%d\t", elfSectionsHeader[i].sh_offset);
        printf("%d\t", elfSectionsHeader[i].sh_size);
        printf("%d\n", elfSectionsHeader[i].sh_type);
    }
}
void PrintSymbols()
{
    Elf32_Shdr *sections = (Elf32_Shdr *)(map_start + elfHeader->e_shoff);
    char *section_names = (char *)(map_start + sections[elfHeader->e_shstrndx].sh_offset);
    printf("index\tvalue\tsection_index\tsection_name\tsymbol_name\n");
    for (int i = 0; i < elfHeader->e_shnum; i++)
    {
        if (sections[i].sh_type == SHT_SYMTAB || sections[i].sh_type == SHT_DYNSYM)
        {
            Elf32_Sym *elfSymbolTable = (Elf32_Sym *)(map_start + sections[i].sh_offset);
            char *symbol_names = (char *)(map_start + sections[sections[i].sh_link].sh_offset);
            int length = sections[i].sh_size / sections[i].sh_entsize;
            char *secName = section_names + sections[i].sh_name;
            for (int j = 0; j < length; j++)
            {
                printf("[%d]\t", j);
                printf("%08x\t", elfSymbolTable[j].st_value);
                printf("%d\t", elfSymbolTable[j].st_shndx);
                printf("%s\t\t", secName);
                printf("%s\n", symbol_names + elfSymbolTable[j].st_name);
            }
            printf("\n");
        }
    }
}

void RelocationTables()
{
    printf("not implemented yet\n");
}

void quit()
{
    if (Currentfd != -1)
    {
        munmap(map_start, fd_stat.st_size);
        close(Currentfd);
    }
    if (debug_mode == 1)
        fprintf(stderr, "quitting\n");
    exit(0);
}
int main(int argc, char **argv)
{
    int c;
    char buff[4];
    struct fun_desc menu[] = {{"Toggle Debug Mode", ToggleDebugMode},
                              {"Examine ELF File", ExamineELF},
                              {"Print Section Names", PrintSection},
                              {"Print Symbols", PrintSymbols},
                              {"Relocation Tables", RelocationTables},
                              {"Quit", quit},
                              {NULL, NULL}};

    while (1)
    {
        printf("\nChoose action:\n");
        for (int i = 0; menu[i].name != NULL; i++)
            printf("%d-%s\n", i, menu[i].name);
        fgets(buff, 4, stdin);
        sscanf(buff, "%d", &c);
        if ((c < 8) & (c > -1))
        {
            map(menu[c].fun);
        }
        else
        {
            printf("Option: %d\nNot Within bounds\n", c);
            exit(0);
        }
    }
}