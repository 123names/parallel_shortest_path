#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>

#include "mympi.h"

typedef struct Pipe{
    int readEnd;
    int writeEnd;
} Pipe;

void Send(struct Pipe* pipe, void* data, ssize_t amount) {
    ssize_t amountRead = 0;
    while(amountRead < amount){
        ssize_t startRead = amountRead;
        amountRead += write(pipe->writeEnd, data + amountRead, amount - amountRead);
        if(amountRead <= startRead){
            printf("Unable to write to pipe");
        }
    }
}


void Receive(struct Pipe* pipe, void* data, ssize_t amount) {
    ssize_t amountRead = 0;
    while(amountRead < amount){
        ssize_t startRead = amountRead;
        amountRead += read(pipe->readEnd, data + amountRead, amount - amountRead);
        if(amountRead <= startRead){
            printf("Unable to write to pipe");
        }
    }
}

struct Pipe CreatePipe(){
    int setup[2];
    if(pipe(setup)==-1){
      fprintf(stderr, "pipe failed, exit\n");
      exit(0);
    }
    struct Pipe newPipe;
    newPipe.readEnd = setup[0];
    newPipe.writeEnd = setup[1];
    return newPipe;
}

typedef struct MPIProcess{
  pid_t* processes;
  struct Pipe** pipes;
  int numProcesses;
  int rank;
} MPIProcess;

struct MPIProcess singleton;

int MPI_Init(int * argc, char*** argv){
  // always keep last item as number of process
  int index = *argc-1;
  singleton.numProcesses = atoi((*argv)[index]);

  singleton.pipes = malloc(sizeof(Pipe) * singleton.numProcesses);
  for(int i = 0; i < singleton.numProcesses; ++i){
    singleton.pipes[i] = malloc(sizeof(Pipe) * singleton.numProcesses);
    for(int j = 0; j < singleton.numProcesses; ++j){
      singleton.pipes[i][j] = CreatePipe();
    }
  }

  singleton.processes = malloc(sizeof(pid_t) * singleton.numProcesses);
  singleton.processes[0] = getpid();
  singleton.rank = 0;

  for(int i=1; i < singleton.numProcesses; ++i){
      pid_t forkResult = fork();
      if(forkResult == 0){
          singleton.rank = i;
          free(singleton.processes);
          break;
      }
      else if(forkResult == -1){
        printf("fork failed\n");
      }
      else{
        singleton.processes[i] = forkResult;
      }
  }
  // remove first and last argument since user will not using it
  *argc -= 1;
  (*argv)[*argc][0] = '\0';

  return MPI_SUCCESS;
}

int MPI_Finalize(){
    for(int i = 1; i < singleton.numProcesses; ++i){
        int result;
        waitpid(singleton.processes[i], &result, 0);
    }
    return MPI_SUCCESS;
}

int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
  if(comm != MPI_COMM_WORLD){
    return MPI_ERR_COMM;
  }
  if(count < 0){
    return MPI_ERR_COUNT;
  }
  if(datatype < 1){
    return MPI_ERR_TYPE;
  }
  //no tag support yet
  if(tag != MPI_ANY_TAG && tag != 0){
    return MPI_ERR_TAG;
  }
  if(dest >= singleton.numProcesses || dest < 0){
    return MPI_ERR_RANK;
  }
  int trueSize = count * datatype;
  struct Pipe pipeToUse = singleton.pipes[singleton.rank][dest];
  Send(&pipeToUse, buf, trueSize);
  return MPI_SUCCESS;
}

int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status){
  if(comm != MPI_COMM_WORLD){
    return MPI_ERR_COMM;
  }
  if(count < 0){
    return MPI_ERR_COUNT;
  }
  if(datatype < 1){
    return MPI_ERR_TYPE;
  }

  //no tag support yet
  if(tag != MPI_ANY_TAG && tag != 0){
    return MPI_ERR_TAG;
  }
  if(source >= singleton.numProcesses || source < 0){
    return MPI_ERR_RANK;
  }
  int trueSize = count * datatype;
  struct Pipe pipeToUse = singleton.pipes[source][singleton.rank];
  Receive(&pipeToUse, buf, trueSize);
  return MPI_SUCCESS;
}

int MPI_Comm_size(MPI_Comm comm, int *size){
    if(comm != MPI_COMM_WORLD){
      return MPI_ERR_COMM;
    }
    *size = singleton.numProcesses;
    return MPI_SUCCESS;
}

int MPI_Comm_rank(MPI_Comm comm, int *rank){
    if(comm != MPI_COMM_WORLD){
      return MPI_ERR_COMM;
    }
    *rank = singleton.rank;
    return MPI_SUCCESS;
}

int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm ){
  if(comm != MPI_COMM_WORLD){
    return MPI_ERR_COMM;
  }
  if(count < 0){
    return MPI_ERR_COUNT;
  }
  if(datatype < 1){
    return MPI_ERR_TYPE;
  }
  int trueSize = count * datatype;
  // if root, send message
  if(singleton.rank==root){
    for(int i =0; i<singleton.numProcesses;i++){
      // send message
      if(i != root){
        struct Pipe pipeToUse = singleton.pipes[root][i];
        Send(&pipeToUse,buffer, trueSize);
      }
    }
  }else{
    struct Pipe pipeToUse = singleton.pipes[root][singleton.rank];
    Receive(&pipeToUse, buffer, trueSize);
  }
  return MPI_SUCCESS;
}

/*
int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
  if(comm != MPI_COMM_WORLD){
    return MPI_ERR_COMM;
  }
  if(count < 0){
    return MPI_ERR_COUNT;
  }
}
*/
