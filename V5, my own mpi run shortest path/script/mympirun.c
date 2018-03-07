#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "mympi.h"
#include <errno.h>

int main(int argc, char * args[]){
  //assume all file in current dir
  char executeableDir[1024]="";
  getcwd(executeableDir, sizeof(executeableDir));
  //printf("Current directory: %s\n", executeableDir);
  // check first part of input arguement
  if(strcmp("-n",args[1]) == 0 || strcmp("-np",args[1]) == 0){
    // collect useful argements for exec

    char* execargs[argc-1];
    int index = 0;
    for(int i=0; i<(argc-3); i++){
      index = i+3;
      //printf("index: %d, size: %d, item: %s\n",i, strlen(args[index]), args[index]);
      execargs[i] = malloc(strlen(args[index]));
      strcpy(execargs[i], args[index]);
    }
    // copy the number of process, put it in the end of array, and then pass to user program
    execargs[argc-3] = malloc(strlen(args[2]));
    strcpy(execargs[argc-3], args[2]);
    // add null pointer indicate the end of args
    execargs[argc-2] = malloc(sizeof(NULL));
    execargs[argc-2] = NULL;
    // double check
    for(int i=0; i<argc-1; ++i){
      printf("main: %s, index: %d\n",execargs[i], i);
    }
    // execute the user file
    execv(execargs[0], &execargs[0]);
    printf("fail to execute user file, error: %d\n", errno);
    return EXIT_FAILURE;
  }
  else{
    printf("Invaild usage of arguement\n");
    return EXIT_FAILURE;
  }
}
