#include <stdlib.h>
#include <stdio.h>
#include <string.h>

char censor(char c)
{
  if (c == '!')
    return '.';
  else
    return c;
}

char *map(char *array, int array_length, char (*f)(char))
{
  char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
  for (int i = 0; i < array_length; i++)
  {
    mapped_array[i] = f(array[i]);
  }
  return mapped_array;
}

char encrypt(char c) /* Gets a char c and returns its encrypted form by adding 3 to its value. 
          If c is not between 0x20 and 0x7E it is returned unchanged */
{
  (c >= 0x20 && c <= 0x7E) ? c = c + 3 : c;
  return c;
}
char decrypt(char c) /* Gets a char c and returns its decrypted form by reducing 3 to its value. 
            If c is not between 0x20 and 0x7E it is returned unchanged */
{
  (c >= 0x20 && c <= 0x7E) ? c = c - 3 : c;
  return c;
}
char cprt(char c) /* If c is a number between 0x20 and 0x7E, cprt prints the character of ASCII value c followed 
                    by a new line. Otherwise, cprt prints the dot ('.') character. After printing, cprt returns 
                    the value of c unchanged. */
{
  (c >= 0x20 && c <= 0x7E) ? printf("%c \n", c) : printf(".\n");
  return c;
}
char my_get(char c) /* Ignores c, reads and returns a character from stdin using fgetc. */
{
  return fgetc(stdin);
}
struct fun_desc
{
  char *name;
  char (*fun)(char);
};
int main(int argc, char **argv)
{
  int base_len = 5, s, c;
  char buff[4];
  char *carray = malloc(base_len);
  char* tmp; 
  carray[0] = '\0';
  struct fun_desc menu[] = {{"Censor", censor}, {"Encrypt", encrypt}, {"Decrypt", decrypt}, {"Print String", cprt}, {"Get String", my_get}, {NULL, NULL}};
  while (1)
  {
    printf("Please choose a function:\n");
    for (int i = 0; menu[i].name != NULL; i++)
      printf("%d) %s\n", i, menu[i].name);
    fgets(buff, 4, stdin);
    s = sscanf(buff, "%d", &c);
    if (c<5 & c> - 1)
    {
      printf("Option: %d\nWithin bounds\n", c);
      tmp = map(carray, 5, menu[c].fun);
      free(carray);
      carray=tmp;
      printf("DONE.\n");
      
    }
    else
    {
      printf("Option: %d\nNot Within bounds\n", c);
      free(carray);
      exit(0);
    }
  }
}