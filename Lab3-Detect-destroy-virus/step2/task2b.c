#include <stdlib.h>
#include <stdio.h>
#include <string.h>
///////////////////////////////////////////////
typedef struct virus
{
  unsigned short SigSize;
  char virusName[16];
  unsigned char *sig;
} virus;
/* 1a */
virus *readVirus(FILE *file)
{
  virus *v = (virus *)malloc(sizeof(virus));
  v->sig = NULL;
  v->SigSize = 0;
  fread(&v->SigSize, sizeof(char), 2, file);
  fread(&v->virusName, sizeof(char), 16, file);
  v->sig = (unsigned char *)calloc(v->SigSize, sizeof(unsigned char));
  fread(v->sig, sizeof(char), v->SigSize, file);

  return v;
}
void printVirus(virus *virus, FILE *output)
{
  if (output == NULL)
  {
    printf("illeggal file");
    return;
  }
  fprintf(output, "%s", "Virus name: ");
  fprintf(output, "%s", virus->virusName);
  fprintf(output, "\nVirus Size: %d", virus->SigSize);
  fprintf(output, "%s", "\nsignature:\n");
  for (int i = 0; i < virus->SigSize; i++)
  {
    fprintf(output, "%02X", (virus->sig)[i]);
    if ((i % 19 == 0) && i > 0)
      fprintf(output, "%s", "\n");
    else
      fprintf(output, "%s", " ");
  }
  fprintf(output, "%s", "\n");
}
void virus_free(virus *vir)
{
  if (vir != NULL)
  {
    free(vir->sig);
    free(vir);
  }
}
/////////////////////////////////////////////
typedef struct link link;
struct link
{
  link *nextVirus;
  virus *vir;
};
/* 1b */

void list_print(link *virus_list, FILE *f)
{ /* Print the data of every link in list to the given stream. Each item followed by a newline character. */
  virus *v = NULL;
  link *ls = NULL;
  if (virus_list == NULL)
  {
    printf("There virus list is empty\n");
    return;
  }
  if (virus_list->vir != NULL)
  {
    v = virus_list->vir;
    printVirus(v, f);
  }
  if (virus_list->nextVirus != NULL)
  {
    ls = virus_list->nextVirus;
    list_print(ls, f);
  }
}

link *list_append(link *virus_list, virus *data)
{
  link *virusLink = malloc(sizeof(link));
  virusLink->vir = data;
  virusLink->nextVirus = NULL;
  if (virus_list == NULL)
  {
    return virusLink;
  }
  else
  {
    link *ptr = virus_list;
    while (ptr->nextVirus != NULL)
      ptr = ptr->nextVirus;
    ptr->nextVirus = virusLink;
  }
  return virus_list;
}

void list_free(link *virus_list) /* Free the memory allocated by the list. */
{
  link *ptr;
  virus *vir = NULL;
  while (virus_list != NULL)
  {
    ptr = virus_list;
    virus_list = virus_list->nextVirus;
    vir = ptr->vir;
    virus_free(vir);
    free(ptr);
  }
}

link *load_signatures(link *sig, FILE *signatureFile) /* loading signatures from input file */
{
  virus *v;
  while (!feof(signatureFile))
  {
    v = readVirus(signatureFile);
    if ((v->SigSize) > 0)
      sig = list_append(sig, v);
    else
    {
      virus_free(v);
    }
  }
  return sig;
}
void print_signatures(link *sig) /* loading signatures from input file */
{
  if (sig == NULL)
    printf("\nNo signatures\n");
  list_print(sig, stdout);
}
////////////////////////////////////////////////////////////
/*1c*/
void detect_virus(char *buffer, unsigned int size, link *virus_list, FILE *output)
{
  virus *vir = NULL;
  link *ptr;
  for (int i = 0; i < size; i++)
  {
    ptr = virus_list;
    while (ptr != NULL)
    {
      vir = ptr->vir;
      if ((vir != NULL) && (vir->SigSize <= (size - i + 1)))
      {
        if (memcmp(&buffer[i], vir->sig, vir->SigSize) == 0)
        {
          fprintf(output, "%s\n", "Virus located!!");
          fprintf(output, "The location: %d\n", i);
          fprintf(output, "The virus name: %s\n", vir->virusName);
          fprintf(output, "The virus size: %d\n", vir->SigSize);
        }
      }
      ptr = ptr->nextVirus;
    }
  }
}
///////////////////////////////////
/*2 */
void kill_virus(char *fileName, int signitureOffset, int signitureSize)
{
  FILE *suspectedFile;
  int flag;
  char *replace = malloc(signitureSize);
  for (int i = 0; i < signitureSize; i++)
  {
    replace[i] = 0x90;
  }
  suspectedFile = fopen(fileName, "r+");
  if (suspectedFile == NULL)
  {
    printf("Failed opening suspected file\n");
  }
  fseek(suspectedFile, signitureOffset, SEEK_SET);
  flag = fwrite(replace, sizeof(char), signitureSize, suspectedFile);
  if (flag == 0)
  {
    printf("Failed fixing file\n");
  }
  else
  {
    printf("file fixed succesfully\n");
  }
  fclose(suspectedFile);
  free(replace);
  fflush(stdin);
}

////////////////////////
int main(int argc, char **argv)
{
  FILE *signatureFile;
  FILE *suspectedFile;
  char path[100];
  link *signatures = NULL;
  int s, c, counter = 0, virusLocation, virusSize;
  char buff[4];
  char buffer[10000];
  char *suspectedFileName;
  char vis[4];
  /*reading the input file*/
  if (argc < 1)
  {
    printf("a legal path name to suspuected file was not given");
    list_free(signatures);
    fclose(signatureFile);
    exit(0);
  }
  for (int x = 0; x < argc; x++)
    if (strncmp(argv[x], " ", 1) != 0)
      suspectedFileName = &argv[x][0];
  /*The menu*/
  while (1)
  {
    printf("1) Load signatures\n");
    printf("2) Print signatures\n");
    printf("3) Detect viruses\n");
    printf("4) Fix file\n");
    fgets(buff, 4, stdin);
    s = sscanf(buff, "%d", &c);
    if (s <= 0)
    {
      printf("the input scan failed - invalid menu input\n");
      exit(0);
    }
    if (c<5 & c> 0)
    {
      if (c == 1)
      {
        printf("Option: %d - enter file name\n", c);
        fgets(path, 100, stdin);
        path[strcspn(path, "\n")] = 0;
        if (s <= 0)
        {
          printf("the input scan failed - invalid adress\n");
          exit(0);
        }
        signatureFile = fopen(path, "r+");
        if (signatureFile == NULL)
        {
          printf("Failed opening file\n");
          exit(0);
        }
        fread(&vis, sizeof(char), 4, signatureFile);
        signatures = load_signatures(signatures, signatureFile);
      }
      else if (c == 2)
        print_signatures(signatures);
      else if (c == 3)
      {
        suspectedFile = fopen(suspectedFileName, "r+");
        if (suspectedFile == NULL)
        {
          printf("Failed opening suspected file\n");
          exit(0);
        }
        while (!feof(suspectedFile))
        {
          fread(&buffer[counter], sizeof(char), 1, suspectedFile);
          counter++;
        }
        if (counter > sizeof(buffer))
          detect_virus(buffer, sizeof(buffer), signatures, stdout);
        else
          detect_virus(buffer, counter, signatures, stdout);
        fclose(suspectedFile);
      }
      else
      {
        char buff2[8];
        printf("Enter the virus location: \n");
        fgets(buff2, sizeof(buff2), stdin);
        sscanf(buff2, "%d", &virusLocation);
        printf("Enter the virus size: \n");
        fgets(buff2, sizeof(buff2), stdin);
        sscanf(buff2, "%d", &virusSize);
        kill_virus(suspectedFileName, virusLocation, virusSize);
      }
    }
    else
    {
      printf("Option: %d\nNot Within bounds\n", c);
      list_free(signatures);
      fclose(signatureFile);
      exit(0);
    }
  }
}