# Number of processes:
N=6

# Change this as you like:
MPIEXEC_OPTS=--oversubscribe

#Compiler and MPI executables
MPIRUN=$(shell which mpirun)

CFLAGS = -Wall -O2 -fopenmp

#Hostfile -> list of cluster nodes 
HOSTFILE = hostfile


#Build main program
main: main.c lib/libbmp/libbmp.c lib/bmp.c
	@mpicc -o $@ $^ -lm

    #Create hostfile
	@/share/ifi/available-nodes.sh > $(HOSTFILE)


#Build main program with openMP
mainMP: main.c lib/libbmp/libbmp.c lib/bmp.c
	@mpicc $(CFLAGS) -o $@ $^ -lm
    #Create hostfile
	@/share/ifi/available-nodes.sh > $(HOSTFILE)


#Run code on cluster
run_parallel: main
	@mpiexec -n $(N) --hostfile $(HOSTFILE) $(MPIEXEC_OPTS) ./main
    #@mpiexec -n $(N) $(MPIEXEC_OPTS) ./main


#Run parallel code with multiple threads
run_openMP: mainMP
	@mpiexec -n $(N) --hostfile $(HOSTFILE) $(MPIEXEC_OPTS) ./mainMP

# 	@mpiexec -np $(N) ./mainMP



#Run code locally
run_local: main
	@mpiexec -n $(N) $(MPIEXEC_OPTS) ./main


clean:
	@rm sobel.bmp
	@rm emboss.bmp
	@rm -f hostfile
	@rm main
	@rm mainMP
	@rm mainMP_openMP


.PHONY: clean
