/*
C program for split the matrix in to different parts
Create binary file that contain part of matrix
*/
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

void split_by_column(int row, int column, int mat[row][column], int totalParts, int parts[row][column/totalParts], int requrePart){
	// if matrix can be divide equally
	if(column%totalParts==0){
		int partSize = column/totalParts;
		for (int i = 0; i < row; i++) {
			for (int j = 0; j < partSize; j++) {
				parts[i][j] = mat[i][(j+requrePart*partSize)];
			}
		}
	}
	// if matrix can't be divide equally
	else{
		printf("The matrix can't be divide equally\n");
	}
}

void split_by_row(int row, int column, int mat[row][column], int totalParts, int parts[row/totalParts][column], int requrePart){
	// if matrix can be divide equally
	if(row%totalParts==0){
		int partSize = row/totalParts;
		for (int i = 0; i < partSize; i++) {
			for (int j = 0; j < column; j++) {
				parts[i][j] = mat[(i+requrePart*partSize)][j];
			}
		}
	}
	// if matrix can't be divide equally
	else{
		printf("The matrix can't be divide equally\n");
	}
}

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
	initial_cost_matrix(totalNodes, cost);
	// fill data in cost matrix
	for(int i =0; i<totallines; i++){
		cost[source[i]][destination[i]] = weight[i];
		cost[destination[i]][source[i]] = weight[i];
	}
	printMatrix(totalNodes, totalNodes, cost);

	// Put splited matrix into binary file
	for(int i =0; i<numParts; i++){
		// generate file name
		FILE * fout;
		char *name = "part";
		char fname [40] = {0};
		sprintf(fname, "%s%d.bin", name, i);
		// split cost matrix
		int parts[totalNodes][totalNodes/numParts];
		// split_by_column(totalNodes, totalNodes, cost, numParts, parts, i);
		// printMatrix(totalNodes, totalNodes/numParts, parts);
		split_by_row(totalNodes, totalNodes, cost, numParts, parts, i);
		printMatrix(totalNodes/numParts, totalNodes, parts);

		// write part to binary file
		fout = fopen(fname, "wb");
		fwrite(&parts, sizeof(parts),1,fout);
		fclose(fout);

	}
	return 0;
}
