#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "mympi.h"

struct MPIProcess{
  pid_t* processes;
  struct Pipe** pipes;
  int numProcesses;
  int rank;
} MPILibrary;

struct MPIProcess skeleton;

int MPI_Init(int *argc, char*** argv){
  if(argc>0){
    printf("%d\n",argc);
    //skeleton.numProcesses = atoi(argv[argc-1]);

    return MPI_SUCCESS;
  }
}
int MPI_Finalize(){
  return MPI_SUCCESS;
}
int MPI_Comm_size(MPI_Comm comm, int *size){
  size = 0;
  return MPI_SUCCESS;
}
int MPI_Comm_rank(MPI_Comm comm, int *rank){
  rank = 0;
  return MPI_SUCCESS;
}
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm){
  if(comm != MPI_COMM_WORLD){
    return MPI_ERR_COMM;
  }
  if(count < 0){
    return MPI_ERR_COUNT;
  }
  if(datatype < 1){
    return MPI_ERR_TYPE;
  }
  return MPI_SUCCESS;
}
int Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status){
  if(comm != MPI_COMM_WORLD){
    return MPI_ERR_COMM;
  }
  if(count < 0){
    return MPI_ERR_COUNT;
  }
  if(datatype < 1){
    return MPI_ERR_TYPE;
  }
  return MPI_SUCCESS;
}
int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm){
  if(comm != MPI_COMM_WORLD){
    return MPI_ERR_COMM;
  }
  if(count < 0){
    return MPI_ERR_COUNT;
  }
  if(datatype < 1){
    return MPI_ERR_TYPE;
  }
  return MPI_SUCCESS;
}
int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm){
  if(comm != MPI_COMM_WORLD){
    return MPI_ERR_COMM;
  }
  if(sendcount < 0||recvcount<0){
    return MPI_ERR_COUNT;
  }
  if(recvtype < 1||sendtype<1){
    return MPI_ERR_TYPE;
  }
  return MPI_SUCCESS;
}
