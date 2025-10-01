#Compiler and MPI executables
CC=gcc
MPICC=$(shell which mpicc)
MPIRUN=$(shell which mpirun)





# Number of processes:
N=6

# Change this as you like:
# MPIEXEC_OPTS=--oversubscribe

#Hostfile -> list of cluster nodes 
HOSTFILE = hostfile

all: main


#Build main program
main: main.c lib/libbmp/libbmp.c lib/bmp.c
	@mpicc -o $@ $^ -lm
#	Create hostfile
	@/share/ifi/available-nodes.sh > $(HOSTFILE)


#Run code on cluster
run_cluster: main
	@echo -e "\nRunning code on cluster...\n"
	@$(MPIRUN) --hostfile $(HOSTFILE) -np $(N) ./main


# 	@$(MPIRUN) --hostfile $(HOSTFILE) -np $(N) $(MPIEXEC_OPTS) ./main


# 	@mpiexec -n $(N) --hostfile $(HOSTFILE) $(MPIEXEC_OPTS) ./main


#Run code locally
run_local: main
	@echo -e "\nMAKEFILE -> 'run_local:' is being run\n"

	@echo -e "\nRunning code locally...\n"

	@mpiexec -n $(N) $(MPIEXEC_OPTS) ./main

# 	@mpiexec -n $(N) $(MPIEXEC_OPTS) ./main


clean:
	@rm main
	@rm -f hostfile

.PHONY: clean











# CC=gcc
# MPICC=$(shell which mpicc)
# MPIRUN=$(shell which mpirun)

# #This line is run if just "make" has been typed in the terminal
# all: mainSequential mainMPI mainMPI_optimized


# mainSequential: mainSequential.c lock.o
# 	@echo -e "\nCompiling the 'mainSequential' file" 
# 	@$(CC) $^ -o $@
# # 	@gcc mainSequential.c lock.o -o mainSequential
	

# mainMPI: mainMPI.c lock.o
# 	@echo -e "\nCompiling the 'mainMPI' file" 
# 	@$(MPICC) $^ -o $@ -lm
# # 	$(shell which mpicc) mainMPI.c lock.o -o mainMPI
	
# mainMPI_optimized: mainMPI_optimized.c lock.o
# 	@echo -e "\nCompiling the 'mainMPI_optimized' file" 
# 	@$(MPICC) $^ -o $@ -lm


# run: mainMPI hostfile
# 	@echo -e "\nRunning mainMPI\n" 
# #	np x = number of processes to launch
# #	Last number = number of lock combinations
# 	@$(MPIRUN) --hostfile hostfile -np 30 ./mainMPI 3


# run_optimized: mainMPI_optimized hostfile
# 	@echo -e "\nRunning mainMPI_optimized\n" 
# #	np "x" = number of processes to launch
# #	Last number = number of lock combinations
# 	@$(MPIRUN) --hostfile hostfile -np 10 ./mainMPI_optimized 3


# run_local: mainMPI 
# #	Launch 4 MPI processes locally to brut force the lock with a sequence of 3 numbers 
# 	@$(MPIRUN) -np 4 ./mainMPI 3 


# clean:
# 	@rm mainSequential mainMPI mainMPI_optimized
# 	@echo "Successfully removed 'mainSequential', 'mainMPI' and 'mainMPI_optimized' executables"


# .PHONY: clean
