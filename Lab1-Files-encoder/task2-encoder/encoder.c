#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
  FILE *ptrout = stdout;
  FILE *ptrin = stdin;
  FILE *ptrerr = stderr;
  char c;
  char *path;
  char *writePath;
  int db = 0, i, counter = 0, sub = 0, add = 0, key = 0, input = 0, output = 0;
  for (i = 1; i < argc; i++)
  {
    (strcmp(argv[i], "-D") == 0) ? db = 1 : db;
    if (strncmp(argv[i], "-i", 2) == 0)
    {
      input = 1;
      path = &argv[i][2];
    }
    if (strncmp(argv[i], "-o", 2) == 0)
    {
      output = 1;
      writePath = &argv[i][2];
    }
    if (strncmp(argv[i], "+e", 2) == 0)
    {
      add = 1;
      key = argv[i][2];
    }
    if (strncmp(argv[i], "-e", 2) == 0)
    {
      sub = 1;
      key = argv[i][2];
    }
  }
  ((key >= '0') && (key <= '9')) ? key = key - '0' : key;
  ((key >= 'A') && (key <= 'F')) ? key = key - 'A' + 10 : key;
  (input == 1) ? ptrin = fopen(path, "r") : 0;
  (output == 1) ? ptrout = fopen(writePath, "w") : 0;
  while ((c = fgetc(ptrin)) != EOF)
  {
    (db == 1) ? fprintf(ptrerr, "%d\t", (int)c) : 0;
    ((add==1)&&(c!='\n'))? c=c+(int)key:c;
    ((sub==1)&&(c!='\n'))? c=c-(int)key:c;
    if ((c >= 'A') && (c <= 'Z') && (add == 0) && (sub == 0))
    {
      c = c + ('a' - 'A');
      counter++;
    }
    (db == 1) ? fprintf(ptrerr, "%d\n", (int)c) : 0;
    ((c == '\n') && (db == 1)) ? fprintf(ptrerr, "\nthe number of letters: %d\n", counter) : 0;
    fputc(c,ptrout);
  }
  return 0;
}