# parallel_shortest_path
//To compile MPI library
cc -c mympi.c
// pack lib comde in archev
ar qc libmpi.a mympi.o
// create mpirun
cc mympirun.c -o mpirun

//To link with MPI library
cc -c <your shortest>.c
// link lib object file to user program object file
cc <your shortest>.o mympi.o -o shortest
// link archeved lib object file to user program object file
cc <your shortest>.o libmpi.a -o shortest

//To run
./mpirun -n <number of processes> ./shortest <your arguments>
