#include <stdio.h>

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

void dijkstra(int, int, int [*][*], int, int);

const int INF = 999;

int main(){
	int part = 2;
	int matrixSize = 8;
	for(int i=0; i<part; i++){
		char *name = "part";
		char fname [40] = {0};
		sprintf(fname, "%s%d.bin", name, i);
		FILE *fin = fopen(fname, "rb");
		int costPart [matrixSize/part][matrixSize];
		fread(costPart, sizeof(costPart),1, fin);
		printMatrix(matrixSize, matrixSize/part, costPart);
		// pass matrix to function
		//dijkstra(matrixSize,matrixSize/part, costPart, 0, 7);
	}
}
void dijkstra(int rowSize,int columnSize, int cost[rowSize][columnSize], int source, int target){
	// keep distance array, 
	int dist[rowSize], pred[rowSize], visited[rowSize];
	int count, mindistance, nextNode;
	// initial distance array from source, and set visited flag to 0
	for(int i =0; i<rowSize; i++){
		dist[i] = cost[source][i];
		pred[i] = source;
		visited[i] = 0;
	}
	// since it start with source, change source flag to 1
	visited[source] = 1;
	count=1;
	printArray(rowSize, dist);
	// loop through all nodes
	while(count<rowSize-1){
		mindistance = INF;
		// find neighborNode that give minimum distance
		for(int i=0; i<rowSize;i++){
			if(dist[i]<mindistance&&!visited[i]){
				mindistance = dist[i];
				nextNode = i;
			}
		}
		// set nextnode flag to 1
		visited[nextNode] = 1;
		// update distance array only when 3 condition meet
		for(int i=0;i<rowSize;i++){
			//1. The nextnode have edge to unvisited neighbor
			if(!visited[i]){
				//2. The distance between nextnode to unvisited neighbor is not INF 
				//(Since i initializzed INF as 99, we do not need to check this condition because last
				//if will check it
				//3. (min-distance of privious node edge + nextnode's all edge) is smaller than 
				//oranginal distance in distance array 
				if(mindistance+cost[nextNode][i]<dist[i]){
					dist[i] = mindistance+cost[nextNode][i];
					pred[i]=nextNode;
				}
			}
		}
		count++;
		printArray(rowSize,dist);
	}
	for(int i=0;i<rowSize;i++){
		if(i!=source){
			printf("\nDistance form node%d to node%d = %d", source, i, dist[i]);
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
