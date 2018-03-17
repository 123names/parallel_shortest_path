#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "mympi.h"

#define MPI_BCAST_TAG -2
#define MPI_GATHER_TAG -3
#define min(x, y) x < y ? x : y

typedef struct Pipe{
    int readEnd;
    int writeEnd;
} Pipe;

typedef struct MessageMetadata{
  int tag;
  ssize_t count;
} MessageMetadata;

typedef struct MessageEntry{
  MessageMetadata metadata;
  void* data;
  struct MessageEntry* next;
} MessageEntry;

struct MPIProcess{
  MessageEntry** backlog;
  pid_t* processes;
  struct Pipe** pipes;
  int numProcesses;
  int rank;
} MPIProcess;

struct MPIProcess singleton;

void Send(struct Pipe* pipe, const void* data, ssize_t amount) {
    ssize_t amountWrite = 0;
    while(amountWrite < amount){
        ssize_t startWrite = amountWrite;
        // write to pipe, if -1, failed
        amountWrite += write(pipe->writeEnd, data + amountWrite, amount - amountWrite);
        if(amountWrite <= startWrite){
            printf("Failed to write to pipe\n");
            exit(0);
        }
    }
}

void Receive(struct Pipe* pipe, void* data, ssize_t amount) {
    ssize_t amountRead = 0;
    while(amountRead < amount){
        ssize_t startRead = amountRead;
        // read from pipe, if -1, failed
        amountRead += read(pipe->readEnd, data + amountRead, amount - amountRead);
        if(amountRead <= startRead){
            printf("Failed to read from pipe\n");
            exit(0);
        }
    }
}

void SendMessage(Pipe* pipe, const void* data, ssize_t count, int tag){
  MessageMetadata metadata;
  metadata.tag = tag;
  metadata.count = count;
  Send(pipe, &metadata, sizeof(MessageMetadata));
  Send(pipe, data, count);
}

void ReceiveMessage(Pipe* pipe, MessageEntry** backlog, void* data, ssize_t count, int tag){
  MessageEntry* prev = (MessageEntry*)0, *entry;
  for(entry = *backlog; entry; prev = entry, entry = entry->next){
    if(tag == MPI_ANY_TAG || tag == entry->metadata.tag){
      if(prev)
        prev->next = entry->next;
      else
        *backlog = entry->next;
      memcpy(data, entry->data, min(entry->metadata.count, count));
      free(entry->data);
      free(entry);
      return;
    }
  }

  if(tag == MPI_ANY_TAG){
    MessageMetadata metadata;
    Receive(pipe, &metadata, sizeof(MessageMetadata));
    Receive(pipe, data, min(metadata.count, count));
  }
  else
    while(1){
      MessageEntry* newEntry = malloc(sizeof(MessageEntry));
      Receive(pipe, &newEntry->metadata, sizeof(MessageMetadata));
      if(newEntry->metadata.tag == tag){
        Receive(pipe, data, min(newEntry->metadata.count, count));
        free(newEntry);
        return;
      }
      newEntry->data = malloc(newEntry->metadata.count);
      Receive(pipe, newEntry->data, newEntry->metadata.count);
      newEntry->next = (MessageEntry*)0;
      if(prev)
        prev->next = newEntry;
      else
        *backlog = newEntry;
      prev = newEntry;
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

int MPI_Init(int * argc, char*** argv){
  // always keep last item as number of process
  int index = *argc-1;
  singleton.numProcesses = atoi((*argv)[index]);
  singleton.backlog = malloc(sizeof(MessageEntry*) * singleton.numProcesses);
  singleton.pipes = malloc(sizeof(Pipe) * singleton.numProcesses);
  for(int i = 0; i < singleton.numProcesses; ++i){
    singleton.pipes[i] = malloc(sizeof(Pipe) * singleton.numProcesses);
    singleton.backlog[i] = (MessageEntry*)0;
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
          singleton.processes = (pid_t*)0;
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
  //printf("Deducting argv");
  *argc -= 1;
  (*argv)[*argc][0] = '\0';

  return MPI_SUCCESS;
}

int MPI_Finalize(){
    for(int i = 0; i < singleton.numProcesses; ++i){
      for(MessageEntry* entry = singleton.backlog[i]; entry;){
        MessageEntry* next = entry->next;
        free(entry->data);
        free(entry);
        entry = next;
      }
      free(singleton.pipes[i]);
      if(singleton.processes){
          int result;
          waitpid(singleton.processes[i], &result, 0);
      }
    }
    free(singleton.backlog);
    free(singleton.pipes);
    if(singleton.processes)
      free(singleton.processes);
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
  if(dest >= singleton.numProcesses || dest < 0){
    return MPI_ERR_RANK;
  }
  if(tag < MPI_ANY_TAG)
    return MPI_ERR_TAG;
  int trueSize = count * datatype;
  struct Pipe pipeToUse = singleton.pipes[singleton.rank][dest];
  SendMessage(&pipeToUse, buf, trueSize, tag);
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
  if(tag < MPI_ANY_TAG)
    return MPI_ERR_TAG;

  if(source >= singleton.numProcesses || source < 0){
    return MPI_ERR_RANK;
  }
  int trueSize = count * datatype;
  struct Pipe pipeToUse = singleton.pipes[source][singleton.rank];
  ReceiveMessage(&pipeToUse, singleton.backlog + source, buf, trueSize, tag);
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
        SendMessage(&pipeToUse,buffer, trueSize, MPI_BCAST_TAG);
      }
    }
  }else{
    struct Pipe pipeToUse = singleton.pipes[root][singleton.rank];
    ReceiveMessage(&pipeToUse, singleton.backlog + root, buffer, trueSize, MPI_BCAST_TAG);
  }
  return MPI_SUCCESS;
}


int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
  if(comm != MPI_COMM_WORLD){
    return MPI_ERR_COMM;
  }
  if(sendcount < 0 || recvcount <0){
    return MPI_ERR_COUNT;
  }
  if(sendtype < 1 || recvcount<1){
    return MPI_ERR_TYPE;
  }
  int blockSize = sendcount * sendtype;
  // send message to root
  Pipe sendpipe = singleton.pipes[singleton.rank][root];
  SendMessage(&sendpipe, sendbuf, blockSize, MPI_GATHER_TAG);
  // merge message if root
  if(singleton.rank==root){
    int totalSize = singleton.numProcesses * blockSize;
    //collect all messages
    for(int i =0; i<singleton.numProcesses;i++){
      void *tempbuf = malloc(blockSize);
      struct Pipe pipeToUse = singleton.pipes[i][root];
      ReceiveMessage(&pipeToUse, singleton.backlog + i, tempbuf, blockSize, MPI_GATHER_TAG);
      memcpy(recvbuf+(i*blockSize), tempbuf, blockSize);
    }
  }
  return MPI_SUCCESS;
}
