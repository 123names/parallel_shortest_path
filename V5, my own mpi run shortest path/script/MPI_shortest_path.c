#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "mympi.h"

const int INF = 999;
const int SIGNAL = 9999;

 void printArray(int size, int *array){
 	for(int i=0; i<size; i++){
 		printf("%d\t", array[i]);
 	}
 	printf("\n");
 }

 void printMatrix(int rowSize, int columnSize, int matrix[rowSize][columnSize]){
 	for(int i =0; i<rowSize; i++){
 		for(int j=0; j<columnSize; j++){
 			printf("%d\t", matrix[i][j]);
 		}
 		printf("\n");
 	}
 	printf("\n");
 }

 int countline(FILE * fin){
 	int counter =0;
 	while(!feof(fin)){
 		char ch = fgetc(fin);
 		if(ch == '\n'){
 			counter++;
 		}
 	}
 	rewind(fin);
 	return counter;
 }

 void readFile(FILE *fin, int *source, int*destination, int *weight){
 	int i =0;
 	while(!feof(fin)){
 		if(fscanf(fin, "%d\t%d\t%d\n", &source[i], &destination[i], &weight[i])==3){
 			i++;
 			continue;
 		}
 		else{
 			printf("file format wrong\n");
 			exit(0);
 		}
 	}
 	fclose(fin);
 }

 void merge(int size, int *source, int *destination, int *merged){
 	memcpy(merged, source, size* sizeof(int));
 	memcpy(merged+size, destination, size* sizeof(int));
 }

 int countNodes(int size, int merged[]){
 	int counter = 0;
 	for(int i =0; i<size; i++){
 		int j =0;
 		for(j=0; j<i; j++){
 			if(merged[i] == merged[j]){
 				break;
 			}
 		}
 		if(i==j){
 			counter++;
 		}
 	}
 	return counter;
 }

 void initial_cost_matrix(int totalNodes, int cost [totalNodes][totalNodes]){
 	for (int i=0; i<totalNodes;i++){
 		for(int j=0; j<totalNodes; j++){
 			if(i==j){
 				cost[i][j] = 0;
 			}
 			else{
 				cost[i][j] = INF;
 			}
 		}
 	}
 }

 void split(int row, int column, int mat[row][column], int totalParts, int parts[row][column/totalParts], int requrePart){
 	// if cost matrix can be divide equally
 	if(row%totalParts==0){
 		int partSize = row/totalParts;
 		for (int i = 0; i < row; i++) {
 			for (int j = 0; j < partSize; j++) {
 				parts[i][j] = mat[i][(j+requrePart*partSize)];
 			}
 		}
 	}
 	// if cost matrix can't be divide equally
 	else{
 	}
 }

 void initialize_distance_array(int partSize, int dist[], int pred[], int visited[]){
 	// initial distance array from source, and set visited flag to 0
 	for(int i =0; i<partSize; i++){
 		dist[i] = INF;
 		pred[i] = -1;
 		visited[i] = 0;
 	}
 }

 void findLocalMin(int local_minium [],int partSize, int dist[partSize], int pred[partSize], int visited[partSize], int partIndex){
 	for(int j=0;j<partSize; j++){
 		if(!visited[j] && dist[j]<local_minium[0]){
 			local_minium[0] = dist[j];
 			local_minium[1] = j+(partIndex*partSize);
 		}
 	}
 }

 void readPart(int rows, int columns, int partIndex, int costPart [rows][columns]){
   char *name = "part";
   char fname [40] = {0};
   sprintf(fname, "%s%d.bin", name, partIndex);
   FILE *fin = fopen(fname, "rb");
   fread(costPart, rows*columns*sizeof(int),1, fin);
 }

 void updateDistArray(int totalNodes,int partSize, int mindistance, int dist_local[], int pred_local[], int visited_local[], int childID, int nextNode){
 	int cost [totalNodes][partSize];
 	readPart(totalNodes, partSize, childID, cost);
 	for(int i=0; i<partSize;i++){
 		if(!visited_local[i]){
 			if(mindistance+cost[nextNode][i]<dist_local[i]){
 				dist_local[i] = mindistance+cost[nextNode][i];
 				pred_local[i] = nextNode;
 			}
 		}
 	}
 }

 void printPath(int source, int target, int numNodes, int dist[], int pred[]){

 	for(int i=0;i<numNodes;i++){
 		if(i==target){
 			printf("\nDistance form node%2d to node%2d = %d", source, i, dist[i]);
 			printf("\nPath=%d",i);
 			int j=i;
 			do{
 				j=pred[j];
 				printf("<-%d",j);
 			}while(j!=source);
 		}
 	}
 	printf("\n");
 }

int main(int argc, char * argv[]){
  int num_procs, rank;
  int fromSource = 0;
  int target = 7;
  int numParts = 1;

  MPI_Init(&argc, &argv);
  if(argc==3){
    fromSource = atoi(argv[1]);
    target = atoi(argv[2]);
  }
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
  numParts = num_procs-1;
  // if main process
  if(rank==0){
    printf("From process (%d) Source: %d, target: %d\n", rank, fromSource,target);
    int totallines, totalNodes = 0;
    // read file
    FILE *fin = fopen("simple_graph.txt", "r");
    totallines = countline(fin);
    int source[totallines], destination[totallines], weight[totallines];	// update local distance array
    readFile(fin, source, destination, weight);
    // count nodes
    int merged[totallines*2];
    merge(totallines, source, destination, merged);
    totalNodes = countNodes(totallines*2, merged);
    // constraction cost matrix
    int cost [totalNodes][totalNodes];
    initial_cost_matrix(totalNodes, cost);
    // fill data in cost matrix
    for(int i =0; i<totallines; i++){
      cost[source[i]][destination[i]] = weight[i];
      cost[destination[i]][source[i]] = weight[i];
    }
    int partSize = totalNodes/numParts;
    // Put splited matrix into binary file
    for(int i =0; i<numParts; i++){
      // generate file name
      FILE * fout;
      char *name = "part";
      char fname [40] = {0};
      sprintf(fname, "%s%d.bin", name, i);
      // split cost matrix
      int parts[totalNodes][totalNodes/numParts];
      split(totalNodes, totalNodes, cost, numParts, parts, i);
      //printMatrix(totalNodes, totalNodes/numParts, parts);
      // write part to binary file
      fout = fopen(fname, "wb");
      fwrite(&parts, sizeof(parts),1,fout);
      fclose(fout);
    }
    // brodcast the partSize to worker for create local distance array
    MPI_Bcast(&partSize, 1, MPI_INT, rank, MPI_COMM_WORLD);
    printf("parent brodcast %d\n",partSize);
    // allocate memory for the result
    int *dist = malloc(totalNodes* sizeof(int));
		int *pred = malloc(totalNodes* sizeof(int));
		int local_minium [2];
		int global_min[2];
		bool flag = true;
		int counter = 0;
    MPI_Status status;
    while(flag){
      if(counter==0){
        global_min[0] = 0;
        global_min[1] = fromSource;
      }
      // brodcast result to child processes
      MPI_Bcast(&global_min, 2, MPI_INT, rank, MPI_COMM_WORLD);
      printf("Parent(%d) send mindistance: %d, node: %d\n", rank, global_min[0], global_min[1]);
      global_min[0] = INF;
      global_min[1] = -1;
      // collect local_minium result from child
      // find global min dist node as next node
      for(int i = 1; i<=numParts; i++){
        MPI_Recv(&local_minium, 2 , MPI_INT,i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        printf("Parent received from %d: local_minium: %d, node: %d\n",i,local_minium[0],local_minium[1]);
        if(local_minium[0]<global_min[0]){
          global_min[0] = local_minium[0];
          global_min[1] = local_minium[1];
        }
      }
      if(global_min[0]==INF){
        // send signal to all child process to collect pred_local
        global_min[0] = SIGNAL;
        global_min[1] = -1;
        MPI_Bcast(&global_min, 2, MPI_INT, rank, MPI_COMM_WORLD);
        printf("Parent(%d) send collect pred signal\n", rank);
        // get pred, dist from child
        for(int i = 1; i<=numParts; i++){
          int pred_local[partSize];
          int dist_local[partSize];
          MPI_Recv(&pred_local, partSize , MPI_INT,i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
          memcpy(pred+((i-1)*partSize), pred_local, sizeof(pred_local));
          MPI_Recv(&dist_local, partSize , MPI_INT,i, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
          memcpy(dist+((i-1)*partSize), dist_local, sizeof(dist_local));
        }
        flag = false;
      }
      else{
        printf("calculated global mindistance: %d, node: %d\n", global_min[0], global_min[1]);
      }
      counter++;
    }
    printArray(totalNodes, dist);
    printArray(totalNodes, pred);
    printPath(fromSource, target, totalNodes, dist,pred);
  }
  // worker process
  else{
    int partSize = -1;
    // receive the partSize from main process
    MPI_Bcast(&partSize, 1, MPI_INT, 0, MPI_COMM_WORLD);
    printf("child %d received %d\n",rank,partSize );
    int totalNodes = numParts*partSize;
    int dist_local[partSize], pred_local[partSize], visited_local[partSize];
    initialize_distance_array(partSize, dist_local, pred_local, visited_local);
    int global_min[2];
    int childID = rank-1;
    while(true){
      // get brodcasted nextnode information
      MPI_Bcast(&global_min, 2, MPI_INT, 0, MPI_COMM_WORLD);
      printf("child %d received distance: %d, from node: %d\n",rank,global_min[0], global_min[1]);
      // if received collect pred signal, send pred to main program
      if(global_min[0]==SIGNAL){
        // write pred to parent
        printArray(partSize, pred_local);
        MPI_Send(&pred_local, partSize, MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD);
        MPI_Send(&dist_local, partSize, MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD);
        break;
      }
      int nextNode = global_min[1];
      int local_node_position = nextNode%partSize;
      // set visited flag for local visited array (need to check process have that part of data or not)
      if(childID*partSize<=nextNode&& nextNode<(childID+1)*partSize){
        // if source
        if(global_min[0]==0){
          dist_local[nextNode] = 0;
        }
        visited_local[local_node_position] = 1;
      }
      // update local distance array
      updateDistArray(totalNodes, partSize, global_min[0], dist_local, pred_local, visited_local, childID, nextNode);
      // find local min
      int local_minium [2];
      local_minium[0] = INF;
      local_minium[1] = -1;
      findLocalMin(local_minium,partSize,dist_local,pred_local,visited_local,childID);
      printf("Child %d\n",rank );
      printArray(partSize, dist_local);
      printArray(partSize, visited_local);
      printArray(partSize, pred_local);
      // write local min to parent, parent calculate global min
      MPI_Send(&local_minium, 2, MPI_INT,0,MPI_ANY_TAG,MPI_COMM_WORLD);
      printf("Child %d send local min dist: %d, node: %d\n", rank, local_minium[0],local_minium[1]);
    }
  }
  MPI_Finalize();
}
