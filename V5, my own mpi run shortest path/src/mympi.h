/*
Define necessay values and public functions
*/
#ifndef MPI_COMM_WORLD

#define MPI_COMM_WORLD ((void *)0)

#define MPI_SUCCESS 0
#define MPI_ERR_COUNT 2
#define MPI_ERR_TYPE 3
#define MPI_ERR_TAG 4
#define MPI_ERR_COMM 5
#define MPI_ERR_RANK 6
#define MPI_ERR_OTHER 10000

#define MPI_DATATYPE_NULL 0
#define MPI_BYTE 1
#define MPI_CHAR sizeof(char)
#define MPI_SHORT sizeof(short)
#define MPI_INT sizeof(int)
#define MPI_LONG sizeof(long)
#define MPI_FLOAT sizeof(float)
#define MPI_DOUBLE sizeof(double)
#define MPI_LONG_DOUBLE sizeof(long double)
#define MPI_SIGNED_CHAR sizeof(signed char)
#define MPI_UNSIGNED_CHAR sizeof(unsigned char)
#define MPI_UNSIGNED_SHORT sizeof(unsigned short)
#define MPI_UNSIGNED_INT sizeof(unsigned int)
#define MPI_UNSIGNED_LONG sizeof(unsigned long)

typedef struct MPI_Status{} MPI_Status;

#define MPI_ANY_TAG -1
#define MPI_STATUS_IGNORE ((MPI_Status*)0)

typedef void* MPI_Comm;
typedef unsigned long MPI_Datatype;

int MPI_Init(int *argc, char ***argv);
int MPI_Finalize(void);
int MPI_Comm_size(MPI_Comm comm, int *size);
int MPI_Comm_rank(MPI_Comm comm, int *rank);
int MPI_Send(void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
int MPI_Bcast(void *buffer, int count, MPI_Datatype datatype, int root, MPI_Comm comm );
int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype, void *recvbuf,int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm);

#endif
