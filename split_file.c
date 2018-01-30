#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printParts(int rowSize, int columnSize, int numParts, int parts[rowSize][columnSize][numParts]){
	for(int i =0; i<numParts; i++){
		for (int j=0; j<rowSize; j++){
			for (int k=0; k<columnSize; k++){
				printf("%5d", parts[j][k][i]);
			}
			printf("\n");
		}
		printf("\n");
	}
}

void printArray(int size, int *array){
	for(int i=0; i<size; i++){
		printf("%d\t", array[i]);
	}
	printf("\n");
}

void printMatrix(int size, int matrix[size][size]){
	for(int i =0; i<size; i++){
		for(int j=0; j<size; j++){
			printf("%d\t", matrix[i][j]);
		}
		printf("\n");
	}
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

void split(int row, int column, int mat[row][column], int totalParts, int parts[][column][totalParts]){
	// if cost matrix can be divide equally
	if(row%totalParts==0){
		int partSize = row/totalParts;
		for (int k=0; k<totalParts; k++){
			for (int i = 0; i < partSize; i++) {
				for (int j = 0; j < column; j++) {
					parts[i][j][k] = mat[(i+k*partSize)][j];
				}
			}
		}
	}
	// if cost matrix can't be divide equally
	else{
	}
}

const int INF = 999;

int main(){
	int numParts = 2;
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
	//printMatrix(totalNodes, cost);
	// split cost matrix
	int parts[totalNodes/numParts][totalNodes][numParts];
	split(totalNodes, totalNodes, cost, numParts, parts);
	printParts(totalNodes/numParts, totalNodes, numParts, parts);
	// Put splited matrix into binary file
	for(int i =0; i<numParts; i++){
		// generate file name
		FILE * fout;
		char *name = "part";
		char fname [40] = {0};
		sprintf(fname, "%s%d.bin", name, i);
		// write part to binary file
		fout = fopen(fname, "wb");
		fwrite(&parts[0][0][i], totalNodes/numParts*totalNodes*sizeof(int),1,fout);
		fclose(fout);
		
	}
	return 0;
}
