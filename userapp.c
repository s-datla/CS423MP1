#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void factorial(int n){
  int temp=1, result=1;
  for(temp=1;temp<=n; temp++){
    result *= temp;
  }
  printf("Result: %d\n", result);
}

int main(int argc, int *argv[])
{

  unsigned long pid = getpid();
  char str[256];
  sprintf(str, "echo '%lu'>/proc/mp1/status", pid);
  system(str);
  int i = 0;
  for(i = 1; i <= 20; i++){
    factorial(i);
  }
  return 0;
}
