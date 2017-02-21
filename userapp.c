#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void factorial(int n){
  int temp=0, result=1;
  for(temp=1;temp<=n; temp++){
    result=result*temp;
  }
  printf("Result: %d\n", result);
}

int main(int argc, int *argv[])
{

  long pid;
  int err;
  pid=(long)getpid();
  FILE *proc_entry_fs;
  proc_entry_fs = fopen("/proc/mp1/status", "w");
  if(proc_entry_fs==NULL){
    printf("failed to open file \n");
  }
  char str[256];
  sprintf(str, "%ld", pid);

  err = fprintf(proc_entry_fs, "%s",str);
  printf("pid=%s err=%d \n",str,err);
  fclose(proc_entry_fs);

  int i = 0;
  for(i = 1; i <= 20; i++){
    factorial(i);
  }
  return 0;
}
