#include <stdio.h>
#include <stdlib.h>

const int INF = 999;

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

void readFile(int rows, int columns, int partIndex, int costPart [rows][columns]){
	char *name = "part";
	char fname [40] = {0};
	sprintf(fname, "%s%d.bin", name, partIndex);
	FILE *fin = fopen(fname, "rb");
	fread(costPart, rows*columns*sizeof(int),1, fin);
}
void findMiniumDist(int dist[], int visited[], int arraySize, int minDisNode, int mindistance){
	// find neighborNode that give minimum distance
	for(int i=0; i<arraySize;i++){
		if(!visited[i]&&dist[i]<mindistance){
			printf("test");
			mindistance = dist[i];
			minDisNode = i;
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

void initialize_distance_array(int totalNodes, int dist[], int pred[], int visited[], int source){
	// initial distance array from source, and set visited flag to 0
	for(int i =0; i<totalNodes; i++){
		dist[i] = INF;
		pred[i] = -1;
		visited[i] = 0;
	}
	dist[source] = 0;
}

void dijkstra(int totalParts,int totalNodes, int source, int dist[], int pred[], int visited[]){
	int partSize = totalNodes/totalParts;
	int mindistance, nextNode, count;
	initialize_distance_array(totalNodes, dist, pred, visited, source);
	count=0;
	// loop through all nodes create full shortest path map
	while(count<totalNodes){
		mindistance = INF;
		for(int i=0; i<totalNodes;i++){
			if(dist[i]<mindistance && !visited[i]){
				mindistance = dist[i];
				nextNode = i;
			}
		}
		/*
		int mindistance[totalParts], mindistanceNode[totalParts];
		for(int k =0; k<totalParts; k++){
			// find minium distance from different part of matrix
			mindistance[k]= INF;
			for(int j=partSize*k;j<(partSize+1)*k; j++){
				if(!visited[j] && dist[j]<mindistance[k]{
					mindistance[k] = dist[j];
					mindistanceNode[k] = i;
				}
			}
		}
		// compare to find minium distance node from return result
		int local_minium = INF;
		int local_minium_node = INF;
		for(int k=0; k<totalParts; k++){
			if(local_mindistance[k]<local_minium){
				local_minium = local_mindistance[k];
				local_minium_node = mindistanceNode[k];
			}
		}
		visited[local_minium_node] = 1;
		*/
		visited[nextNode] = 1;
		// update distance array
		for(int k=0; k<totalParts; k++){
			int cost [totalNodes][partSize];
			readFile(totalNodes, totalNodes/totalParts, k, cost);
			//printMatrix(totalNodes, totalNodes/totalParts, cost);
			for(int j=0; j<partSize;j++){
				if(!visited[j+(k*partSize)]){
					if(mindistance+cost[nextNode][j]<dist[j+(k*partSize)]){
						dist[j+(k*partSize)] = mindistance+cost[nextNode][j];
						pred[j+(k*partSize)] = nextNode;
					}
				}
			}
		}
		count++;
		printArray(totalNodes,dist);
		printArray(totalNodes,visited);
		//printArray(totalNodes, pred);
	}
}

int main(int argc, char *argv[]){
	int part = 2;
	int source = 1;
	int target = 5;
	if(argc==3) {
		source = atoi(argv[1]);
		target = atoi(argv[2]);
    }
	int totalNodes = 8;
	int dist[totalNodes], pred[totalNodes], visited[totalNodes];
	
	// pass distance array to function
	dijkstra(part,totalNodes, source, dist, pred, visited);
	// print result path
	printPath(source, target, totalNodes, dist,pred);
}
