#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void split(int row, int column, int mat[row][column], int totalParts, int parts[row/totalParts][column], int requrePart){
	// if cost matrix can be divide equally
	if(row%totalParts==0){
		int partSize = row/totalParts;
		for (int i = 0; i < partSize; i++) {
			for (int j = 0; j < column; j++) {
				parts[i][j] = mat[(i+requrePart*partSize)][j];
			}
		}
	}
	// if cost matrix can't be divide equally
	else{
	}
}

void dijkstra(int rowSize,int columnSize, int cost[rowSize][columnSize], int source){
	// keep distance array, 
	int dist[columnSize], pred[columnSize], visited[columnSize];
	int count, mindistance, nextNode;
	// initial distance array from source, and set visited flag to 0
	for(int i =0; i<columnSize; i++){
		dist[i] = INF;
		visited[i] = 0;
	}
	// since it start with source, change source flag to 1
	dist[source]=0;
	count=0;
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

int main(){
	int totallines, totalNodes = 0;
	// read file
	FILE *fin = fopen("simple_graph.txt", "r");
	totallines = countline(fin);
	int source[totallines], destination[totallines], weight[totallines];
	readFile(fin, source, destination, weight);
	// count nodes
	int merged[totallines*2];
	merge(totallines, source, destination, merged);
	totalNodes = countNodes(totallines*2, merged);
	// constraction cost matrix
	int cost [totalNodes][totalNodes];
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
	// fill data in cost matrix
	for(int i =0; i<totallines; i++){
		cost[source[i]][destination[i]] = weight[i];
		cost[destination[i]][source[i]] = weight[i];
	}
	printMatrix(totalNodes, totalNodes, cost);
	dijkstra(totalNodes,totalNodes, cost, 3);
}
