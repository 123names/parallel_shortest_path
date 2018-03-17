//https://stackoverflow.com/questions/13867809/how-are-mpi-scatter-and-mpi-gather-used-from-c
//#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

#include "mympi.h"

int main(int argc, char **argv) {
    int size, rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Status status;
    int *globaldata=NULL;
    int localdata;

    if (rank == 0) {
        globaldata = malloc(size * sizeof(int));
        for (int i=0; i<size-1; i++)
            globaldata[i] = 2*i+1;

        printf("Initial: Processor %d has data: ", rank);
        for (int i=0; i<size-1; i++){
          printf("%d ", globaldata[i]);
        }
        printf("\n");
        for (int i=0; i<size-1; i++){
          printf("Program sending %d\n", globaldata[i]);
          MPI_Send(globaldata + i, 1, MPI_INT,(i+1),MPI_ANY_TAG,MPI_COMM_WORLD);
        }
    }
    else{
      MPI_Recv(&localdata, 1 , MPI_INT,0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
      printf("Processor %d has data %d\n", rank, localdata);
      localdata *= 2;
      printf("Processor %d doubling the data, now has %d\n", rank, localdata);
    }

    printf("%d, r: %d\n", localdata, rank);
    MPI_Gather(&localdata, 1, MPI_INT, globaldata, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank == 0) {
        printf("Final: Processor %d has data: ", rank);
        for (int i=0; i<size; i++)
            printf("%d ", globaldata[i]);
        printf("\n");
    }

    if (rank == 0){
      free(globaldata);
    }
    MPI_Finalize();
    return 0;
}
