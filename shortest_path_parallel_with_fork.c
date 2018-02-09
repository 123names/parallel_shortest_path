#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>

const int INF = 999;
const int SIGNAL = 9999;
struct Local_Min_Node{
	int local_minium_dist;
	int local_minium_node;
};

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

void readPart(int rows, int columns, int partIndex, int costPart [rows][columns]){
	char *name = "part";
	char fname [40] = {0};
	sprintf(fname, "%s%d.bin", name, partIndex);
	FILE *fin = fopen(fname, "rb");
	fread(costPart, rows*columns*sizeof(int),1, fin);
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
void findLocalMin(struct Local_Min_Node *item,int partSize, int dist[partSize], int pred[partSize], int visited[partSize], int partIndex){
	for(int j=0;j<partSize; j++){
		if(!visited[j] && dist[j]<item->local_minium_dist){
			item->local_minium_dist = dist[j];
			item->local_minium_node = j+(partIndex*partSize);
		}
	}
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

int main(int argc, char *argv[]){
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
	int numParts = 2;
	int fromSource = 0;
	int target = 7;
	if(argc==4) {
		numParts = atoi(argv[1]);
		fromSource = atoi(argv[2]);
		target = atoi(argv[3]);
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

	// fork create mutiple process for find local_mindistance
	// create pipe first, two pipe send result from parent to child with nextNode and send distance array part from child to parent
	int pcfd[numParts][2];
	int cpfd[numParts][2];
	for(int i=0; i<numParts; i++){
		if(pipe(pcfd[i])==-1){
			fprintf(stderr, "pipe: %d failed\n",i);
			exit(0);
		}
		if(pipe(cpfd[i])==-1){
			fprintf(stderr, "pipe: %d failed\n",i);
			exit(0);
		}
	}
	// fork processes
	pid_t main = getpid();
	pid_t children [numParts];
	int childID = -1;
	printf("Parent: %d\n", main);

	for(int i = 0; i<numParts; i++){
		pid_t forkResult = fork();
		if(forkResult<0){
			printf("fork failed\n");
			exit(0);
		}
		else if (forkResult==0){
			childID = i;
			break;
		}
		else{
			children[i] = forkResult;
		}
	}

	// main process calculate minium distance node
	if(main==getpid()){
		int *dist = malloc(totalNodes* sizeof(int));
		int *pred = malloc(totalNodes* sizeof(int));
		struct Local_Min_Node node;
		int global_min[2];
		bool flag = true;
		int counter = 0;
		// global[0] is mindistance, global[1] is nextNode
		while(flag){
			if(counter==0){
				global_min[0] = 0;
				global_min[1] = fromSource;
			}
			// brodcast result to child processes
			for(int i = 0; i<numParts; i++){
				// close read end of pipe
				close(pcfd[i][0]);
				// write result to pipe
				write(pcfd[i][1], &global_min, sizeof(global_min));
				printf("Parent(%d) send mindistance: %d, node: %d\n", getpid(), global_min[0], global_min[1]);
			}
			global_min[0] = INF;
			global_min[1] = -1;
			// collect local_minium result from child
			// find global min dist node as next node
			for(int i = 0; i<numParts; i++){
				close(cpfd[i][1]);
				read(cpfd[i][0],&node,sizeof(node));
				printf("Parent (%d) received: local_minium: %d, node: %d\n",getpid(),node.local_minium_dist,node.local_minium_node);
				if(node.local_minium_dist<global_min[0]){
					global_min[0] = node.local_minium_dist;
					global_min[1] = node.local_minium_node;
				}
			}
			if(global_min[0]==INF){
				// send signal to all child process to collect pred_local
				global_min[0] = SIGNAL;
				global_min[1] = -1;
				for(int i = 0; i<numParts; i++){
					// write result to pipe
					write(pcfd[i][1], &global_min, sizeof(global_min));
					printf("Parent(%d) send collect pred signal\n", getpid());
				}
				// get pred, dist from child
				for(int i = 0; i<numParts; i++){
					int pred_local[partSize];
					int dist_local[partSize];
					read(cpfd[i][0],&pred_local,sizeof(pred_local));
					memcpy(pred+(i*partSize), pred_local, sizeof(pred_local));
					read(cpfd[i][0],&dist_local,sizeof(dist_local));
					memcpy(dist+(i*partSize), dist_local, sizeof(dist_local));
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
	else{
		//current process is a child
		int global_min[2];
		int dist_local[partSize], pred_local[partSize], visited_local[partSize];
		initialize_distance_array(partSize, dist_local, pred_local, visited_local);
		while(true){
			// read data first, close wirting end of pipe
			// get nextNode information
			close(pcfd[childID][1]);
			read(pcfd[childID][0], &global_min, sizeof(global_min));
			printf("Child %d (%d) received mindistance: %d, node: %d\n", childID, getpid(), global_min[0], global_min[1]);
			// if received collect pred signal, send pred to main program
			if(global_min[0]==SIGNAL){
				// write pred to parent
				printArray(partSize, pred_local);
				close(cpfd[childID][0]);
				write(cpfd[childID][1],&pred_local,sizeof(pred_local));
				write(cpfd[childID][1],&dist_local,sizeof(dist_local));
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
			struct Local_Min_Node node, *nodePointer;
			nodePointer = &node;
			node.local_minium_dist = INF;
			node.local_minium_node = -1;
			findLocalMin(nodePointer,partSize,dist_local,pred_local,visited_local,childID);
			printArray(partSize, dist_local);
			printArray(partSize, visited_local);
			printArray(partSize, pred_local);
			// write local min to parent, parent calculate global min
			close(cpfd[childID][0]);
			write(cpfd[childID][1],&node,sizeof(node));
			printf("Child %d(%d) send value: %d\n", childID,getpid(), node);
		}
	}
	return 0;
}
