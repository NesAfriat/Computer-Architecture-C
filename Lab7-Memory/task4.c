#include <stdlib.h>
#include <stdio.h>
#include <string.h>
int digit_cnt(char* input);
int main(int argc, char **argv)
{
  char *buff;
  printf("%d\n",digit_cnt(argv[1]));
  exit(0);
}
int digit_cnt(char* input)
{ 
 int counter=0;
 int i=0;
  while(*(input+i)!='\0') 
  {if(input[i]>='0'&& input[i]<='9')
  counter++;
  i++;
  }
  return counter;
}








































































