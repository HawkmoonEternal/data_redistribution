
all: mpi_test_expand mpi_test_shrink

redist.o: redist.c
	gcc -c redist.c -o redist.o

redist_mpi.o: redist_mpi.c
	gcc -c redist_mpi.c -o redist_mpi.o

mpi_test_expand.o: mpi_test_expand.c
	gcc -c mpi_test_expand.c -o mpi_test_expand.o

mpi_test_expand: mpi_test_expand.o redist_mpi.o redist.o
	mpicc mpi_test_expand.o redist_mpi.o redist.o -o mpi_test_expand -L/opt/hpc/install/ompi -lmpi

mpi_test_shrink.o: mpi_test_shrink.c
	gcc -c mpi_test_shrink.c -o mpi_test_shrink.o

mpi_test_shrink: mpi_test_shrink.o redist_mpi.o redist.o
	mpicc mpi_test_shrink.o redist_mpi.o redist.o -o mpi_test_shrink -L/opt/hpc/install/ompi -lmpi

clean:
	rm -f *.o mpi_test_expand mpi_test_shrink
