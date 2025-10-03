#Compiler and MPI executables
MPIRUN=$(shell which mpirun)

CFLAGS = -Wall -O2 -fopenmp


# Number of processes:
N=6

# Change this as you like:
MPIEXEC_OPTS=--oversubscribe

#Hostfile -> list of cluster nodes 
# HOSTFILE = hostfile

all: main


#Build main program
main: main.c lib/libbmp/libbmp.c lib/bmp.c
	@mpicc -o $@ $^ -lm

#	With openMP
# 	@mpicc $(CFLAGS) -o $@ $^ -lm

#	Create hostfile
# 	@/share/ifi/available-nodes.sh > $(HOSTFILE)


#Run code on cluster
run_cluster: main
# 	@mpiexec -n $(N) --hostfile $(HOSTFILE) $(MPIEXEC_OPTS) ./main

	@echo -e "\nRunning code on cluster...\n"
	@$(MPIRUN) -np $(N) ./main

# 	@$(MPIRUN) --hostfile $(HOSTFILE) -np $(N) ./main


# 	@$(MPIRUN) --hostfile $(HOSTFILE) -np $(N) $(MPIEXEC_OPTS) ./main




#Run code locally
run_local: main
	@echo -e "\nMAKEFILE -> 'run_local:' is being run\n"

	@echo -e "\nRunning code locally...\n"

	@mpiexec -n $(N) $(MPIEXEC_OPTS) ./main

# 	@mpiexec -n $(N) $(MPIEXEC_OPTS) ./main


clean:
	@rm main
	@rm -f hostfile
	@rm emboss.bmp
	@rm sobel.bmp


.PHONY: clean
