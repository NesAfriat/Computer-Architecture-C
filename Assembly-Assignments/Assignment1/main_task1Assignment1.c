#include <stdio.h>
#include <stdlib.h>
#define	MAX_LEN 32			/* maximal input string size */
					/* enough to get 32-bit string + '\n' + null terminator */
extern void assFunc(int x);
char c_checkValidity(int x){
  
  if(x%2 == 0){return 1;}
  return 0;
}
int main(int argc, char** argv)
{
  int input = 0;
  char buf[MAX_LEN];
  if(fgets(buf, MAX_LEN, stdin) < 0){
    printf("fail to read input\n");
    exit(0);
  }		
  sscanf(buf,"%d",&input);
  assFunc(input);			/* call your assembly function */

  return 0;
}